// #include <mobo/jit.h>
#include <arpa/inet.h>
#include <capstone/capstone.h>
#include <fcntl.h>
#include <mobo/kvm.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#define RAMSIZE (16 * 1024l * 1024l)

static inline uint64_t __attribute__((always_inline)) rdtsc(void) {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return lo | ((uint64_t)(hi) << 32);
}

using namespace mobo;

std::atomic<int> nruns = 0;
std::atomic<int> nhcalls = 0;

std::mutex dirty_lock;
std::mutex clean_lock;
std::queue<kvm *> clean;
std::queue<kvm *> dirty;
std::condition_variable dirty_signal;

static void add_dirty(kvm *v) {
  dirty_lock.lock();
  dirty.push(v);
  dirty_lock.unlock();
  dirty_signal.notify_one();
}

int kvmfd = 0;

static kvm *get_clean(std::string &path, size_t memsize) {
  {
    std::scoped_lock lck(clean_lock);
    if (!clean.empty()) {
      auto v = clean.front();
      clean.pop();
      return v;
    }
  }
  // allocate a new one
  kvm *v = new kvm(kvmfd, path, 1);
  v->init_ram(memsize);
  v->reset();
  return v;
}

void set_affinity(int cpu) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
}

auto cleaner(void) {
  while (1) {
    std::unique_lock<std::mutex> lk(dirty_lock);

    dirty_signal.wait(lk);
    // grab something to clean
    //
    kvm *v = nullptr;

    if (!dirty.empty()) {
      v = dirty.front();
      dirty.pop();
    }
    lk.unlock();
    if (v != nullptr) {
      v->reset();
      // printf("RESET!\n");
      clean_lock.lock();
      clean.push(v);
      clean_lock.unlock();
    }
  }
}

void print_throughput(float sleep_time = 0.25) {
  float secs = 0;

#define US_IN_S (1000 * 1000)

  long sleep_us = sleep_time * US_IN_S;

  long last_ran = nruns.load();
  long last_hcalls = nhcalls.load();

  while (1) {
    usleep(sleep_us);
    secs += sleep_time;

    auto ran = nruns.load();       // atomic load of global variable
    auto hcalls = nhcalls.load();  // atomic load of global variable

    auto rps = (long)((ran - last_ran) / sleep_time);
    auto hcps = (long)((hcalls - last_hcalls) / sleep_time);
    printf("%ld runs, %ld hcalls\n", rps, hcps);
    last_ran = ran;

    last_hcalls = hcalls;
  }
}

/*
auto runner_1(int i, std::string path) {
  while (1) {
    auto v = get_clean(path, RAMSIZE);

    v->run();

    nruns++;

    add_dirty(v);
  }
  return 0;
}



int test_throughput_1(std::string path, int nrunners, int ncleaners) {
  std::vector<std::thread> cleaners;
  for (int i = 0; i < ncleaners; i++) cleaners.emplace_back(cleaner);


  for (int i = 0; i < 25; i++) {
    clean.push(get_clean(path, RAMSIZE));
  }


  std::vector<std::thread> runners;
  for (int i = 0; i < nrunners; i++)
    runners.emplace_back(runner_1, i, path);

  print_throughput();


  // join all the threads
  for (auto &t : runners) t.join();
  for (auto &t : cleaners) t.join();

  return 0;
}
*/

bool send_all(int socket, void *buffer, size_t length) {
  char *ptr = (char *)buffer;
  while (length > 0) {
    int i = write(socket, ptr, length);
    if (i < 1) return false;
    ptr += i;
    length -= i;
  }
  return true;
}

class tcp_workload : public workload {
 public:
  int socket = 0;

  tcp_workload(int sock) : socket(sock) {}

  virtual int handle_hcall(struct kvm_regs &regs, size_t ramsize, void *ram) {
    nhcalls++;

    // send
    if (regs.rax == 2) {
      off_t buf_off = regs.rdi;
      size_t len = regs.rsi;

      if (buf_off + len >= ramsize) {
        regs.rax = -1;
        return WORKLOAD_RES_OKAY;
      }

      char *buf = (char *)ram + buf_off;
      regs.rax = send_all(socket, buf, len);
      return 0;
    }

    // recv
    if (regs.rax == 3) {
      off_t buf_off = regs.rdi;
      size_t len = regs.rsi;

      if (buf_off + len >= ramsize) {
        regs.rax = -1;
        return WORKLOAD_RES_OKAY;
      }

      char *buf = (char *)ram + buf_off;
      regs.rax = recv(socket, buf, len, 0);
    }
    return WORKLOAD_RES_OKAY;
  }
};

std::queue<int> sockets;

std::mutex socket_lock;
std::condition_variable socket_signal;

static void add_sock(int sock) {
  socket_lock.lock();
  sockets.push(sock);
  socket_lock.unlock();
  socket_signal.notify_one();
}

void runner_2(kvm *vm, int id) {
  set_affinity(id);

  while (1) {
    std::unique_lock<std::mutex> lk(socket_lock);

    if (sockets.empty()) socket_signal.wait(lk);

    if (sockets.empty()) continue;

    // printf("remaining: %ld\n", sockets.size());
    int socket = sockets.front();
    sockets.pop();

    lk.unlock();

    tcp_workload conn(socket);
    // run the vm
    vm->run(conn);

    close(socket);


    auto tsc_buf = (uint64_t*)vm->mem_addr(0x1000);
    uint64_t base = tsc_buf[0];
    for (int i = 1; tsc_buf[i] != 0; i++) {
      printf("%zu\n", tsc_buf[i] - base);
    }


    nruns++;

    // reset the vm
    vm->reset();
  }
}

#define MAX 80
#define PORT 8000
#define BACKLOG 1000 /* how many pending connections queue will hold */

void run_server(void) {
  int server_fd;
  struct sockaddr_in my_addr;     /* my address information */
  struct sockaddr_in client_addr; /* connector's address information */

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  bzero((char *)&my_addr, sizeof(my_addr));
  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port = htons(PORT);       /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */

  int optval = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
             sizeof(int));

  if (bind(server_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) ==
      -1) {
    perror("bind");
    exit(1);
  }

  if (listen(server_fd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  while (1) {
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int fd;
    if ((fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size)) ==
        -1) {
      perror("accept");
      continue;
    }

#define CONN_DEBUG

#ifdef CONN_DEBUG

    struct hostent *hostp;

    hostp = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr,
                          sizeof(client_addr.sin_addr.s_addr), AF_INET);

    char *hostaddrp = inet_ntoa(client_addr.sin_addr);
    printf("connected (%d): %s (%s)\n", nruns.load(), hostp->h_name, hostaddrp);
#endif
    add_sock(fd);
  }
}

int test_throughput_2(std::string path, int nrunners) {
  int nprocs = get_nprocs();

  size_t ramsize = 1 * 1024l * 1024l;
  std::vector<std::thread> runners;

  for (int i = 0; i < nrunners; i++) {
    runners.emplace_back(runner_2, get_clean(path, ramsize), i % nprocs);
  }

  set_affinity(nrunners + 1);

  // start the server
  run_server();

  for (auto &t : runners) t.join();
  return -1;
}

int main(int argc, char **argv) {
  if (argc == 1) return -1;
  if (kvmfd == 0) kvmfd = open("/dev/kvm", O_RDWR);

  int nprocs = get_nprocs();

  int c;
  while ((c = getopt(argc, argv, "t:")) != -1) switch (c) {
      case 't':
        nprocs = atoi(optarg);
        break;
      default:
        printf("unexected flag '%c'\n", c);
        exit(1);
    }

  printf("nprocs=%d\n", nprocs);
  // lele
  // signal(SIGPIPE, SIG_IGN);

  test_throughput_2(argv[optind], nprocs);
}
