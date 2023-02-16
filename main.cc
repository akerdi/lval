#include <stdio.h>
#include "config.h"

#ifdef USE_MY_COMPILER
  #include <Compiler.h>
#endif

int main(int argc, char** argv) {
  printf("%s VERSION: %d.%d\n", argv[0], APP_VERSION_MAJOR, APP_VERSION_MINOR);
#ifndef USE_MY_COMPILER
  printf("not implement compiler yet!\n");
  return 1;
#else
  printf("giao: %s\n", (char*)compiler("111"));
#endif
  return 0;
}