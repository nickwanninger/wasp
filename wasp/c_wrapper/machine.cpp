#include <wasp/compiler_defs.h>
#include <wasp/machine.h>
#include <wasp/workload.h>
#include <wasp_c/loader.h>

#include <wasp_c/machine.h>
#include "opaque_types.h"


#include <map>
#include <deque>
#include <mutex>


struct machine_cache {
	std::mutex cached_lock;
	size_t memsize = 0;
	std::deque<wasp_machine_t *> machines;

	auto *get(void) {
		cached_lock.lock();

		wasp_machine_t *m = NULL;
		if (machines.size() == 0) {
			m = (wasp_machine_t *) malloc(sizeof(wasp_machine_t));
			auto vm = wasp::machine::create(memsize);
			m->container = new wasp::wrapper::details::machine_container(vm);
			// printf("construct new!\n");
		} else {
			m = machines.front();
			machines.pop_front();
			// printf("was cached!\n");
		}
		cached_lock.unlock();

		return m;
	}

	auto put(wasp_machine_t *m) {
		cached_lock.lock();
		machines.push_front(m);
		cached_lock.unlock();
	}
};

static std::map<size_t, struct machine_cache> cached_machines;
std::mutex cached_lock;

static struct machine_cache &get_cache(size_t memsize) {
	cached_lock.lock();
	auto &mc = cached_machines[memsize];
	mc.memsize = memsize;
	cached_lock.unlock();

	return mc;
}

extern "C" {

wasp_machine_t *wasp_machine_create(size_t memsize) {
	auto &mc = get_cache(memsize);

	return mc.get();
}

void wasp_machine_free(wasp_machine_t *self, size_t memsize) {
  PANIC_IF_NULL(self);
	get_cache(memsize).put(self);
}

void wasp_machine_run(wasp_machine_t *self, wasp_workload_t *workload) {
  PANIC_IF_NULL(self);
  PANIC_IF_NULL(workload);

  wasp::wrapper::details::workload_wrapper wrapper(workload);
  self->container->get().run(wrapper);
}

void wasp_machine_reset(wasp_machine_t *self) {
  PANIC_IF_NULL(self);
  self->container->get().reset();
}


int wasp_fib(int n) {
		if (n < 2) return n;
		return wasp_fib(n-2) + wasp_fib(n-1);
}

}; // extern "C"





class null_workload : public wasp::workload {
public:
  null_workload() = default;
  ~null_workload() override = default;
  int handle_hcall(wasp::regs_t &regs, size_t ramsize, void *ram) override {
		printf("rax=%016llx\n", regs.rax);
		return WORKLOAD_RES_OKAY;
	}
};



/**
 * arg is loaded into 0x0000 and is read back out when running is done
 */
extern "C" void wasp_run_virtine(const char *code, size_t codesz, size_t memsz, void *arg, size_t argsz) {
	auto vm = wasp_machine_create(memsz);
	wasp_inject_code(vm, (void*)code, codesz, 0x1000);

  // wasp::wrapper::details::workload_wrapper wrapper(workload);
	auto &machine = vm->container->get();

	auto *arg_pos = (void*)machine.gpa2hpa(0);

	// copy the argument into the machine's ram
	memcpy(arg_pos, arg, argsz);

	// for (int i = 0; i < 32; i++) printf("%02x ", ((unsigned char *)arg_pos)[i]); printf("\n");

	null_workload wl;
	machine.run(wl);

	// for (int i = 0; i < 32; i++) printf("%02x ", ((unsigned char *)arg_pos)[i]); printf("\n");

	// copy the argument out of the machine's ram
	memcpy(arg, arg_pos, argsz);
}
