#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "traps.h"

#define N 10

struct trace_event {
    int pid;
    char command_name[16];   // name of the command/process
    char syscall_name[16];   // name of the syscall
    int retval;              // return value
};

static struct trace_event events[N];
static int event_index = 0; 
static struct spinlock tracelock;

static int total_recorded_events = 0;

void
traceinit(void)
{
    initlock(&tracelock, "trace");
}

// void
// record_trace_event(int pid, const char *cmd_name, const char *sys_name, int retval)
// {

//     acquire(&tracelock);
//     events[event_index].pid = pid;
//     safestrcpy(events[event_index].command_name, cmd_name, sizeof(events[event_index].command_name));
//     safestrcpy(events[event_index].syscall_name, sys_name, sizeof(events[event_index].syscall_name));
//     events[event_index].retval = retval;

//     event_index = (event_index + 1) % N;
//     release(&tracelock);
// }

void
record_trace_event(int pid, const char *cmd_name, const char *sys_name, int retval)
{
    acquire(&tracelock);
    events[event_index].pid = pid;
    safestrcpy(events[event_index].command_name, cmd_name, sizeof(events[event_index].command_name));
    safestrcpy(events[event_index].syscall_name, sys_name, sizeof(events[event_index].syscall_name));
    events[event_index].retval = retval;

    event_index = (event_index + 1) % N;
    if (total_recorded_events < N) {
        total_recorded_events++;
    }
    release(&tracelock);
}


// int
// sys_stracedump(void)
// {
//     acquire(&tracelock);
//     for (int i = 0; i < N; i++) {
//         struct trace_event *e = &events[i];
//         if (e->pid != 0) {
//             if (e->retval == -2)
//             {
//                 cprintf("EVENT %d: pid=%d, command_name=%s, syscall=%s\n",
//                     i, e->pid, e->command_name, e->syscall_name);
//             }
//             else {
//             cprintf("EVENT %d: pid=%d, command_name=%s, syscall=%s, return_value=%d\n",
//                     i, e->pid, e->command_name, e->syscall_name, e->retval);
//             }
//         }
//     }
//     release(&tracelock);
//     return 0;
// }


int
sys_stracedump(void)
{
    acquire(&tracelock);

    // Determine how many events to print
    int count = total_recorded_events < N ? total_recorded_events : N;

    // The oldest event is at (event_index - count) mod N
    int start_index = (event_index - count + N) % N;

    for (int i = 0; i < count; i++) {
        int idx = (start_index + i) % N;
        struct trace_event *e = &events[idx];

        if (e->pid != 0) {
            if (e->retval == -2) {
                cprintf("EVENT %d: pid=%d | command_name = %s | syscall=%s\n",
                    i, e->pid, e->command_name, e->syscall_name);
            } else {
                cprintf("EVENT %d: pid=%d | command_name = %s | syscall=%s | return value=%d\n",
                    i, e->pid, e->command_name, e->syscall_name, e->retval);
            }
        }
    }

    release(&tracelock);
    return 0;
}
