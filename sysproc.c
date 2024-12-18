#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_strace(void)
{
    int mode; // 1 = enable, 0 = disable

    if (argint(0, &mode) < 0) {
        return -1; // Invalid argument
    }

    // cprintf("sys_strace: mode = %d\n", mode);

    struct proc *curproc = myproc();
    curproc->strace_on = mode; // Set the tracing flag

    // cprintf("curproc->strace_on = %d\n", curproc->strace_on);
    return 0;
}

int
sys_strace_filter(void)
{
    char *filter;
    if (argstr(0, &filter) < 0)
        return -1;


    struct proc *curproc = myproc();
    safestrcpy(curproc->strace_filter, filter, sizeof(curproc->strace_filter));

    // cprintf("curproc->strace_filter = %s and filter = %s\n", curproc->strace_filter, filter);
    return 0;
}

