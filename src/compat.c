#include <stddef.h>
#include <types.h>
#include <genesis.h>

// types.h causes a conflict somehow...
#ifndef int8_t
#define int8_t char
#endif

// hal_write implementation for now...
int hal_write(int fd, const void *buf, int nbytes) {
  static int8_t line = 20;
  static char str[38];
  int len = 37;
  int remaining = nbytes+1;

  uint8_t start;
  const char* bufcpy = (char*)buf;

  do {
    start = bufcpy;
    len = 37;
    if(line > 26) line = 20;
    if(remaining < 38) len = remaining;

    // fd values are just ignored
    snprintf(str, len, "%s", bufcpy);
    VDP_drawText("                                     ", 1, line);
    VDP_drawText(str, 1, ++line);

    bufcpy += 37;
    remaining -= 37;
  } while (strlen(start) > 37);
  return 0;
}

// hal_init unimplemented for now...
void hal_init(void){
}
