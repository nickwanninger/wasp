#include "printk.h"

static inline void outl(short port, int val) {
  __asm__ volatile("outl %0, %1" ::"a"(val), "dN"(port));
}

extern void print(const char *);
extern void exit(int);

// def in hypervisor.asm
extern int write(int fd, void *buf, int len);

extern int send(void *buf, int len);
extern int recv(void *buf, int len);

unsigned long strlen(const char *c) {
  int n = 0;
  for (; c[n] != '\0'; n++)
    ;
  return n;
}

int send_string(char *s) { return send(s, strlen(s)); }

#define BUFSIZE 1000
char buf[BUFSIZE];

#define HTTP_200 "HTTP/1.0 200 OK\r\n\r\n"
int kmain(void) {
  send_string(HTTP_200);

  for (int i = 0; i < 100; i++) {
    snprintk(buf, BUFSIZE, "%d is a number!\n", i);
    send_string(buf);
  }

  exit(0);

  return 0;
}
