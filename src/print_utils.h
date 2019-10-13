#ifndef __PRINT_UTILS_H__
#define __PRINT_UTILS_H__

#include <Arduino.h>

void printfTo(Stream& stream, const char *format, ...) {
  va_list ap;
  char buffer[128];
  va_start(ap, format);
  int n = vsnprintf(&buffer[0], sizeof(buffer), format, ap);
  va_end(ap);
  if (n >= 0) {
    stream.println(buffer);
  }
}

void print_zero_hex(Stream& stream, int data) {
  if (data < 16) {
    stream.print("0");
  }
  stream.print(data, HEX);
}

#endif
