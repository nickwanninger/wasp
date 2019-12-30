#include "printk.h"

static inline void outl(short port, int val) {
  __asm__ volatile("outl %0, %1" ::"a"(val), "dN"(port));
}

extern void exit(int);

// def in hypervisor.asm
extern int write(int fd, void *buf, int len);

extern int send(void *buf, int len);
extern int recv(void *buf, int len);


extern void record_timestamp(void);

unsigned long strlen(const char *c) {
  int n = 0;
  for (; c[n] != '\0'; n++)
    ;
  return n;
}

int send_string(char *s) { return send(s, strlen(s)); }


#define BUFSZ (4096 * 6)
char content_buf[BUFSZ];



int send_data(void *data, int len) {
  // uses content_buf
  int written = sprintk(content_buf, "HTTP/1.1 200 OK\r\n" \
                                           "Content-Length: %d\r\n\r\n", len);


  char *content = content_buf + written;
  char *source = data;

  for (int i = 0; i < len; i++) {
    content[i] = source[i];
  }

  written += len;


  send(content_buf, written);
  return 0;
}

#define CRNL "\r\n"

char *msg = "<html><body><b>hello</b> world</body></html>\n";


char recv_buf[1000];



void kmain(void) {

  // IN MAIN
  record_timestamp();
  int nrecv = recv(recv_buf, 1000);

  // AFTER RECV
  record_timestamp();
  record_timestamp();
  record_timestamp();
  record_timestamp();
  record_timestamp();
  record_timestamp();


  send_data(recv_buf, nrecv);

  // AFTER SEND
  record_timestamp();

  exit(0);
}
