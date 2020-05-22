#include <wasp/compiler_defs.h>
#include <wasp/machine.h>
#include <wasp/workload.h>

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

}; // extern "C"
