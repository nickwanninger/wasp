#include <fcntl.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <elfio/elfio.hpp>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "compiler_defs.h"
#include "mobo/workload.h"
#include "platform/getopt.h"
#include "platform/platform.h"
#include "platform/unistd.h"

#define RAMSIZE (16 * 1024l * 1024l)

using namespace mobo;

class binary_loader {
 public:
  binary_loader() {}
  virtual ~binary_loader() {}

  virtual bool inject(mobo::machine &vm) = 0;
};

class elf_loader : public binary_loader {
  ELFIO::elfio reader;

 public:
  elf_loader(std::string path) { reader.load(path); }

  virtual bool inject(mobo::machine &vm) override {
    auto entry = reader.get_entry();

    auto secc = reader.sections.size();
    for (int i = 0; i < secc; i++) {
      auto psec = reader.sections[i];
      auto type = psec->get_type();
      if (psec->get_name() == ".comment") continue;

      if (type == SHT_PROGBITS) {
        auto size = psec->get_size();
        if (size == 0) continue;
        const char *data = psec->get_data();
        auto dst = (char *)vm.gpa2hpa(psec->get_address());
        memcpy(dst, data, size);
      }
    }

    struct regs_t r;
    vm.cpu(0).read_regs_into(r);
    r.rip = entry;
    vm.cpu(0).write_regs(r);

    // XXX: should we do this?
    struct regs_special_t sr;
    vm.cpu(0).read_regs_special_into(sr);
    sr.cs.base = 0;
    sr.ds.base = 0;
    vm.cpu(0).write_regs_special(sr);

    return true;
  }
};

class flatbin_loader : public binary_loader {
  std::string path;

 public:
  explicit flatbin_loader(std::string path) : path(std::move(path)) {}

  bool inject(mobo::machine &vm) override {
    auto entry = 0x0000;

    void *addr = vm.gpa2hpa(entry);
  	if (addr == nullptr) {
  	  PANIC("failed to convert GPA -> HPA, got nullptr");
  	}

    FILE *fp = fopen(path.data(), "r");
    if (fp == nullptr) {
      PANIC("failed to open binary\n");
    }

    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    fread(addr, sz, 1, fp);
    fclose(fp);

    struct regs_t r = {};
    vm.cpu(0).read_regs_into(r);
    r.rip = entry;
    r.rsp = 0x1000;
    r.rbp = 0x1000;
    vm.cpu(0).write_regs(r);

    // XXX: should we do this?
//    struct regs_special_t sr;
//    vm.cpu(0).read_sregs(sr);
//    sr.cs.base = 0;
//    sr.ds.base = 0;
//    vm.cpu(0).write_regs_special(sr);

    return true;
  }
};

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

machine::ptr create_machine(const std::string &path, size_t memsize);

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
  machine::ptr v = create_machine(path, memsize);
  return v;
}

machine::ptr create_machine(const std::string &path, size_t memsize) {
  machine::ptr v = platform::create(PLATFORM_ANY);
  v->allocate_ram(memsize);
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

  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
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

    nruns++;

    // reset the vm
    vm.reset();
  }
}
#pragma clang diagnostic pop

#define MAX 80
#define PORT 8080
#define BACKLOG 1000 /* how many pending connections queue will hold */

void run_server() {
  zn_socket_t server_fd;
  struct sockaddr_in my_addr = {};     /* my address information */
  struct sockaddr_in client_addr = {}; /* connector's address information */

  zn_socket_init();
  if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    throw std::runtime_error("failed to create socket");
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
    throw std::runtime_error("failed to bind to socket");
    exit(1);
  }

  if (listen(server_fd, BACKLOG) == -1) {
    throw std::runtime_error("failed to listen to socket");
    exit(1);
  }

  while (true) {
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int fd;
    if ((fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size)) ==
        -1) {
      throw std::runtime_error("failed to accept connection on socket");
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
    fprintf(stderr, "created machine #%d\n", i);
  }

  zn_set_affinity(nrunners + 1);

  // start the server
  run_server();

  for (auto &t : runners) t.join();
  return -1;
}

class double_workload : public workload {
  int val = 20;

  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                   void *ram) override {
    if (regs.rax == 0) {
//      printf("%s: rax = 0\n", __FUNCTION__);
      regs.rbx = val;
      return WORKLOAD_RES_OKAY;
    }

    if (regs.rax == 1) {
//      printf("%s: rax = 1\n", __FUNCTION__);
      if (regs.rbx != val * 2) {
        throw std::runtime_error("double test failed\n");
      }
      return WORKLOAD_RES_OKAY;
    }

//    printf("%s: rax = %lld (kill)\n", __FUNCTION__, regs.rax);
    return WORKLOAD_RES_KILL;
  }
};

class fib_workload : public workload {
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                   void *ram) override {
    printf("rax=%ld\n", regs.rax);
    return WORKLOAD_RES_KILL;
  }
};

static int boot_runc = 0;

class boottime_workload : public workload {
 public:
  boottime_workload(void) {
    if (boot_runc == 0) {
      printf("Platform, cli, lgdt, prot, in 32, id map, long, in 64\n");
    }
    boot_runc++;
  }
  ~boottime_workload(void) { printf("\n"); }
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize,
                   void *ram) override {
    if (regs.rax == 1) {
      uint64_t *tsc = (uint64_t *)ram;

#ifdef __linux__
      printf("KVM Noserial, ");
#endif

#ifdef _WIN32
      printf("Hyper-V, ");
#endif

      uint64_t baseline = tsc[0];
      // uint64_t overhead = tsc[1] - baseline;

      for (int i = 2; tsc[i] != 0; i++) {
        auto prev = tsc[i - 1];
        auto curr = tsc[i];

        printf("%4ld", curr - prev);
        if (tsc[i + 1] != 0) printf(",");
      }
      /*
      for (int i = 2; tsc[i] != 0; i++) {
        printf("%4ld", tsc[i] - baseline);
        if (tsc[i + 1] != 0) printf(",");
      }
      */
      return WORKLOAD_RES_KILL;
    }

    return WORKLOAD_RES_OKAY;
  }
};

template <class W, class L>
bool run_test(std::string path, int run_count = 1,
              const char *stdout_path = nullptr) {
  printf("test [%s]\n", path.data());

  L loader(path);

  int ofd = -1;

  if (stdout_path != nullptr) {
    ofd = dup(1);
    int new_fd = open(stdout_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    dup2(new_fd, 1);
    close(new_fd);
  }

  auto start = std::chrono::high_resolution_clock::now();

  machine::ptr vm = create_machine(path, 1 * 1024l * 1024l);
  for (int i = 0; i < run_count; i++) {
    W work;
    vm->reset();
    loader.inject(*vm);
    vm->run(work);
  }
  auto end = std::chrono::high_resolution_clock::now();

  if (ofd != -1) {
    dup2(ofd, 1);
    close(ofd);
  }

  printf(
      "    DONE %fms\n",
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start) *
          1000.0);

  return true;
}

int main(int argc, char **argv) {
  run_test<double_workload, flatbin_loader>("build/tests/double64.bin");
  // run_test<double_workload, elf_loader>("build/tests/double.elf");

  exit(0);

  run_test<fib_workload, flatbin_loader>("build/tests/fib20.bin");
  // run_test<fib_workload, elf_loader>("build/tests/fib20.elf");

  run_test<boottime_workload, flatbin_loader>("build/tests/boottime.bin", 1000,
                                              "data/boottime.csv");

  if (argc <= 1) {
    fprintf(stderr, "usage: mobo [kernel.elf]\n");
    return -1;
  }

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

  nprocs = 1;
  test_throughput_2(argv[optind], nprocs);
  //  auto machine = create_machine(argv[optind], 1);
  printf("success!");
  getchar();
}
