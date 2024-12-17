#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf(2, "Usage: child_reader filename\n");
    exit();
  }
  int fd = open(argv[1], 0); // read-only
  if(fd < 0) {
    printf(2, "Failed to open %s\n", argv[1]);
    exit();
  }

  char buf[100];
  int n = read(fd, buf, sizeof(buf)-1);
  if(n < 0) {
    printf(2, "Read error\n");
  } else {
    buf[n] = '\0';
    printf(1, "Read from file: %s\n", buf);
  }
  
  close(fd);
  exit();
}
