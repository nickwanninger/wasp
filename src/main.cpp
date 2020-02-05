#include <fcntl.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "compiler_defs.h"
#include "mobo/workload.h"
#include "platform/getopt.h"
#include "platform/platform.h"
#include "platform/unistd.h"

#define RAMSIZE (16 * 1024l * 1024l)

using namespace mobo;

std::atomic<int> nruns = 0;
std::atomic<int> nhcalls = 0;

std::mutex dirty_lock;
std::mutex clean_lock;
std::queue<mobo::machine::ptr> clean;
std::queue<mobo::machine::ptr> dirty;
std::condition_variable dirty_signal;

static void add_dirty(mobo::machine::ptr v) {
  dirty_lock.lock();
  dirty.push(v);
  dirty_lock.unlock();
  dirty_signal.notify_one();
}

static std::shared_ptr<mobo::machine> get_clean(std::string &path,
                                                size_t memsize) {
  {
    std::scoped_lock lck(clean_lock);
    if (!clean.empty()) {
      auto v = clean.front();
      clean.pop();
      return v;
    }
  }
  // allocate a new one
  mobo::machine::ptr v = mobo::platform::create(PLATFORM_ANY);
  v->allocate_ram(memsize);
  v->load_elf(path);
  v->reset();
  return v;
}

auto cleaner(void) {
  while (true) {
    std::unique_lock<std::mutex> lk(dirty_lock);

    dirty_signal.wait(lk);
    // grab something to clean
    //
    mobo::machine::ptr v;

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

  int sleep_us = sleep_time * US_IN_S;

  long last_ran = nruns.load();
  long last_hcalls = nhcalls.load();

  while (true) {
    zn_sleep_micros(sleep_us);
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
  zn_socket_t socket = {};

  tcp_workload(zn_socket_t sock) : socket(sock) {}

  int handle_hcall(struct mobo::regs &regs, size_t ramsize,
                   void *ram) override {
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

std::queue<zn_socket_t> sockets;

std::mutex socket_lock;
std::condition_variable socket_signal;

// lock around showing tsc data
std::mutex data_lock;

int run_count = 0;

static void add_sock(int sock) {
  socket_lock.lock();
  sockets.push(sock);
  socket_lock.unlock();
  socket_signal.notify_one();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void runner_2(mobo::machine::ptr vmp, int id) {
  auto &vm = *vmp;
  zn_set_affinity(id);

  while (true) {
    std::unique_lock<std::mutex> lk(socket_lock);

    if (sockets.empty()) socket_signal.wait(lk);

    if (sockets.empty()) continue;

    // printf("remaining: %ld\n", sockets.size());
    zn_socket_t socket = sockets.front();
    sockets.pop();

    lk.unlock();

    tcp_workload conn(socket);
    // run the vm
    vm.run(conn);

    zn_close_socket(socket);

    /*

    auto tsc_buf = (uint64_t *)vm.gpa2hpa(0x1000);
    data_lock.lock();

    printf("%d, ", run_count++);

    u64 baseline = tsc_buf[0];
    for (int i = 1; tsc_buf[i] != 0; i++) {
      u64 tsc = tsc_buf[i];

      printf("%zu", tsc - baseline);
      if (tsc_buf[i + 1] != 0) printf(", ");
    }
    printf("\n");
    data_lock.unlock();
    */

    nruns++;

    // reset the vm
    vm.reset();
  }
}
#pragma clang diagnostic pop

#define MAX 80
#define PORT 8000
#define BACKLOG 1000 /* how many pending connections queue will hold */

void run_server() {
  zn_socket_t server_fd;
  struct sockaddr_in my_addr = {};     /* my address information */
  struct sockaddr_in client_addr = {}; /* connector's address information */

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  memset((void *)&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port = htons(PORT);       /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */

  int optval = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval,
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

  while (true) {
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

    // char *hostaddrp = inet_ntoa(client_addr.sin_addr);
    // printf("connected (%d): %s (%s)\n", nruns.load(), hostp->h_name,
    // hostaddrp);
#endif
    add_sock(fd);
  }
}

int test_throughput_2(std::string path, int nrunners) {
  int nprocs = zn_get_processors_count();

  size_t ramsize = 1 * 1024l * 1024l;
  std::vector<std::thread> runners;

  for (int i = 0; i < nrunners; i++) {
    runners.emplace_back(runner_2, get_clean(path, ramsize), i % nprocs);
  }

  zn_set_affinity(nrunners + 1);

  // start the server
  run_server();

  for (auto &t : runners) t.join();
  return -1;
}

int main(int argc, char **argv) {
  if (argc == 1) return -1;

  int nprocs = zn_get_processors_count();

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
