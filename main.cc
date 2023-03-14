#include <stdio.h>
#include "config.h"

#ifdef USE_MY_COMPILER
  #include <compiler.h>
#endif

int main(int argc, char** argv) {
  printf("%s VERSION: %d.%d\n", argv[0], APP_VERSION_MAJOR, APP_VERSION_MINOR);
#ifdef USE_MY_COMPILER
  compiler();
#else
  printf("not implement compiler yet!\n");
  return 1;
#endif
  return 0;
}