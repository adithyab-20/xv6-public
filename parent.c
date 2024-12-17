#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf(2, "Usage: parent_spawner filename\n");
    exit();
  }

  // Enable tracing
  strace(1);

  int pid = fork();
  if(pid == 0) {
    // Child process: run child_reader on the given file
    char *child_args[] = {"child_reader", argv[1], 0};
    exec("child_reader", child_args);
    printf(2, "exec failed\n");
    exit();
  } else if(pid > 0) {
    // Parent waits for child to finish
    wait();
    exit();
  } else {
    printf(2, "fork failed\n");
    exit();
  }
}
