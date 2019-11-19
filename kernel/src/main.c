#include "printk.h"

static inline void outl(short port, int val) {
  __asm__ volatile("outl %0, %1" ::"a"(val), "dN"(port));
}

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

unsigned x;  // The state can be seeded with any value.
// Call next() to get 32 pseudo-random bits, call it again to get more bits.
// It may help to make this inline, but you should see if it benefits your
// code.
inline unsigned next(void) {
  unsigned z = (x += 0x6D2B79F5UL);
  z = (z ^ (z >> 15)) * (z | 1UL);
  z ^= z + (z ^ (z >> 7)) * (z | 61UL);
  return z ^ (z >> 14);
}

#define BUFSIZE 1000
char buf[BUFSIZE];

#define CRNL "\r\n"


static int send_http(int status, void *data, int len) {
  if (status == 200) {
    send_string("HTTP/1.1 200 OK" CRNL);
  } else {
    return -1;
  }


  char len_buf[50];
  sprintk(len_buf, "Content-Length: %lu" CRNL, len);
  send_string(len_buf);

  send_string(CRNL);  // start sending data

  send(data, len);

  return 0;
}

void kmain(void) {

  recv(buf, BUFSIZE);
  char *payload = "<html><body><b>hello</b> world</body></html>\n";
  send_http(200, payload, strlen(payload));

  exit(0);
}
