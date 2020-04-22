#pragma once

#include <wasp/workload.h>

class tcp_workload : public wasp::workload {
public:
  zn_socket_t socket;
  explicit tcp_workload(zn_socket_t sock);
  int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override;
  void handle_exit() override;
};
