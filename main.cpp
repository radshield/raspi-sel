#include <csignal>
#include <cstdio>
#include <unistd.h>

static volatile bool sentinel = true;

void int_handler(int signum) {
  sentinel = false;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s MODEL_FILE TARGET_PID\n", argv[0]);
    return -1;
  }

  signal(SIGINT, int_handler);

  while (sentinel) {
    usleep(10);
  }

  return 0;
}