#define __packed __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))

#include <asm.h>
#include <idt.h>
#include <mem.h>
#include <paging.h>
#include <printk.h>
#include <serial.h>
#include <types.h>

extern int kernel_end;

u64 fib(u64 n) {
  if (n < 2) return 1;

  return fib(n - 1) + fib(n - 2);
}

static u64 strlen(char *str) {
  u64 i = 0;
  for (i = 0; str[i] != '\0'; i++)
    ;
  return i;
}


// in src/arch/x86/sse.asm
extern void enable_sse();






int kmain(u64 hypermagic, u64 *hypertable) {
  serial_install();
  init_idt();

  enable_sse();

  printk("hypermagic=%p\n", (void*)hypermagic);
  printk("hypertable=%p\n", (void*)hypertable);


  u64 *table = hypertable;

  printk("%p\n", table[0]);
  printk("%p\n", table[1]);

  int regionc = table[1];


  struct region {
    u64 addr, size;
  } *regions = (void*)(table + 2);

  for (int i = 0; i < regionc; i++) {
    printk("%d: %p %zu\n", i, regions[i].addr, regions[i].size);
  }

  printk("hello, from the kernel\n");


  // simply hltspin
  while (1) halt();
  return 0;
}

