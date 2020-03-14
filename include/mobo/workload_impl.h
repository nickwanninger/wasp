#include "platform/socket.h"
#include "workload.h"

namespace mobo::workload_impl {

class boottime_workload : public mobo::workload {
public:
  boottime_workload();
  ~boottime_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};

class double_workload : public mobo::workload {
  int val;

public:
  double_workload();
  ~double_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};

class fib_workload : public mobo::workload {
public:
  fib_workload();
  ~fib_workload() override;
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
};


class tcp_workload : public workload {
public:
  zn_socket_t socket;
  explicit tcp_workload(zn_socket_t sock);
  int handle_hcall(struct mobo::regs_t &regs, size_t ramsize, void *ram) override;
  void handle_exit() override;
};

}
