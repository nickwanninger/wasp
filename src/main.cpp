#include <fcntl.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "timeit.h"
#include "compiler_defs.h"
#include "mobo/workload.h"
#include "mobo/workload_impl.h"
#include "platform/getopt.h"
#include "platform/platform.h"
#include "platform/unistd.h"

#define RAMSIZE (16 * 1024l * 1024l)

using namespace mobo;
using namespace mobo::workload_impl;

TIMEIT_START(g_main);

std::atomic<int> nruns = 0;
std::atomic<int> nhcalls = 0;

std::mutex dirty_lock;
std::mutex clean_lock;
std::mutex create_lock;
std::queue<mobo::machine::ptr> clean;
std::queue<mobo::machine::ptr> dirty;
std::condition_variable dirty_signal;

static void add_dirty(mobo::machine::ptr v) {
  dirty_lock.lock();
  dirty.push(v);
  dirty_lock.unlock();
  dirty_signal.notify_one();
}

machine::ptr create_machine(size_t memsize);

template <class L>
static std::shared_ptr<mobo::machine> get_clean(
    const std::string &path, size_t memsize)
{
  TIMEIT_FN(g_main);

  {
    std::scoped_lock lck(clean_lock);
    if (!clean.empty()) {
      auto v = clean.front();
      clean.pop();
      return v;
    }
  }

  {
    std::scoped_lock lock(create_lock);
    L loader(path);
    machine::ptr v = create_machine(memsize);
    loader.inject(*v);
    return v;
  }
}

machine::ptr create_machine(size_t memsize) {
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


std::queue<zn_socket_t> sockets;

std::mutex socket_lock;
std::condition_variable socket_signal;

// lock around showing tsc data
std::mutex data_lock;

int run_count = 0;

static void add_sock(int sock) {
  TIMEIT_FN(g_main);
  socket_lock.lock();
  sockets.push(sock);
  socket_lock.unlock();
  socket_signal.notify_one();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void runner_2(int id, const std::string &path, size_t ramsize) {
  zn_set_affinity(id);
  fprintf(stdout, "[tid %d] creating machine #%d\n", std::this_thread::get_id(), id);
  auto vm = get_clean<loader::elf_loader>(path, ramsize);

  while (true) {
    std::unique_lock<std::mutex> lk(socket_lock);

    if (sockets.empty()) socket_signal.wait(lk);

    if (sockets.empty()) continue;

    // printf("remaining: %ld\n", sockets.size());
    zn_socket_t socket = sockets.front();
    sockets.pop();

    lk.unlock();

    tcp_workload conn(socket);
    vm->run(conn);

    nruns++;

    // reset the vm
    vm->reset();
  }
}
#pragma clang diagnostic pop

#define MAX 80
#define PORT 9090
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
    if ((fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
      throw std::runtime_error("failed to accept connection on socket");
      continue;
    }
    TIMEIT_MARK(g_main, "accept");

//#define CONN_DEBUG

#ifdef CONN_DEBUG

    char hostname[NI_MAXHOST] = {};
    int ret = getnameinfo(
        reinterpret_cast<const SOCKADDR *>(&client_addr),
        sizeof(struct sockaddr),
        hostname, sizeof(hostname),
        nullptr,
        0,
        0);

    char host_ip[25] = {};
    if (inet_ntop(AF_INET, &client_addr.sin_addr, host_ip, sizeof(host_ip)) != nullptr) {
      printf("connected (%d): %s (%s)\n", nruns.load(), hostname, host_ip);
    }
    else {
      printf("connected (%d): %s (unknown ip)\n", nruns.load(), hostname);
    }

#endif
    add_sock(fd);
  }
}

int test_throughput_2(const std::string& path, int nrunners) {
  zn_set_affinity(nrunners + 1);

  int nprocs = zn_get_processors_count();

  size_t ramsize = 1 * 1024l * 1024l;
  std::vector<std::thread> runners;

  for (int i = 0; i < nrunners; i++) {
    runners.emplace_back(runner_2, i % nprocs, path, ramsize);
  }

  // start the server
  run_server();

  for (auto &t : runners) t.join();
  return -1;
}

template <class W, class L>
bool run_test(std::string path, int run_count = 1,
              const char *stdout_path = nullptr) {
  printf("test [%s]\n", path.data());

  L loader(path);

  int ofd = -1;

  if (stdout_path != nullptr) {
    ofd = dup(1);
    // int new_fd = open(stdout_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    FILE* file = fopen(stdout_path, "w+");
  	if (file == nullptr)
  	{
        fprintf(stderr, "failed to open file\n");
        return false;
  	}

    int new_fd = fileno(file);
    dup2(new_fd, 1);
    // close(new_fd);
    fclose(file);
  }

  auto start = std::chrono::high_resolution_clock::now();

  // TODO: Use the RAM size from what you're loading or throw if the loader
  // requested memory size is greater than the limit
  machine::ptr vm = create_machine(1 * 1024l * 1024l);
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
//  run_test<double_workload, loader::flatbin_loader>("build/tests/double64.bin");
//  run_test<double_workload, loader::elf_loader>("build/tests/double64.elf");
//
//  run_test<fib_workload, loader::flatbin_loader>("build/tests/fib20.bin");
//  run_test<fib_workload, loader::elf_loader>("build/tests/fib20.elf");

	
//  exit(0);


    run_test<boottime_workload, loader::flatbin_loader>("build/tests/boottime.bin");

    getchar();
    exit(0);
	
  run_test<boottime_workload, loader::flatbin_loader>("build/tests/boottime.bin", 1000,
                                              "data/boottime.csv");

//  if (argc <= 1) {
//    fprintf(stderr, "usage: mobo [kernel.elf]\n");
//    return -1;
//  }

  // int nrunners = zn_get_processors_count();
  //
  // int c;
  // while ((c = getopt(argc, argv, "t:")) != -1) switch (c) {
  //     case 't':
  //       nrunners = atoi(optarg);
  //       break;
  //     default:
  //       printf("unexected flag '%c'\n", c);
  //       exit(1);
  //   }
  //
  // printf("nprocs=%d\n", nrunners);
  // lele
  // signal(SIGPIPE, SIG_IGN);

//  nrunners = 1;
//  test_throughput_2(argv[optind], nprocs);
//  test_throughput_2("./build/kernel.elf", nrunners);
  //  auto machine = create_machine(argv[optind], 1);
  printf("success!");
  getchar();
}
