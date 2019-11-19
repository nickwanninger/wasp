static inline void outl(short port, int val) {
  __asm__ volatile("outl %0, %1" ::"a"(val), "dN"(port));
}



extern void print(const char *);
extern void exit(int);



int fib (int n) {
  if (n < 1) return n;

  return fib(n-1) + fib(n-2);
}
int kmain(void) {
    exit(0);
}
