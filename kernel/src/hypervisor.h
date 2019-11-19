#pragma once


#define OP_WRITE 1
#define OP_READ 2

struct file_op {
  int mode;
  int len;
  int buf;
};
