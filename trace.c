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

#define BUFFER_SIZE 1000

struct trace_event {
    int pid;
    char command_name[16];   // name of the command/process
    char syscall_name[16];   // name of the syscall
    int retval;              // return value
};

// struct buffered_trace {
//     int pid;
//     char command_name[16];
//     char syscall_name[16];
//     int retval;
//     int is_command_end;  // Flag to indicate if this is the last trace for a command
// };

// struct buffered_trace {
//     int pid;                 // Process ID
//     int ppid;               // Parent Process ID
//     char command_name[16];   // Command name
//     char syscall_name[16];   // System call name
//     int retval;             // Return value
//     int is_command_end;     // Flag to indicate if this is the last trace
//     int trace_index;        // Index to maintain order
// };

struct buffered_trace {
    int pid;                 // Process ID
    int ppid;               // Parent Process ID
    char command_name[16];   // Command name
    char syscall_name[16];   // System call name
    int retval;             // Return value
    int trace_index;        // Index to maintain order
};



static struct trace_event events[N];
static int event_index = 0; 
static struct spinlock tracelock;
static int total_recorded_events = 0;

// static struct buffered_trace buffered_events[BUFFER_SIZE];
// static int buffer_index = 0;
// static struct spinlock bufferlock;
// static int current_command_pid = 0; 

// static struct buffered_trace buffered_events[BUFFER_SIZE];
// static int buffer_index = 0;
// static struct spinlock bufferlock;
// static int trace_sequence = 0;  // Global sequence counter for trace ordering

static struct buffered_trace buffered_events[BUFFER_SIZE];
static int buffer_index = 0;
static struct spinlock bufferlock;
static int trace_sequence = 0;


void
traceinit(void)
{
    initlock(&tracelock, "trace");
    initlock(&bufferlock, "buffer");
}

// // Helper function for string comparison
// int
// strncmp(const char *p, const char *q, uint n)
// {
//   while(n > 0 && *p && *p == *q)
//     n--, p++, q++;
//   if(n == 0)
//     return 0;
//   return (uchar)*p - (uchar)*q;
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


static int
is_exit_syscall(const char *sys_name)
{
    return strncmp(sys_name, "exit", 4) == 0;
}


// static int
// is_wait_syscall(const char *sys_name)
// {
//     return strncmp(sys_name, "wait", 4) == 0;
// }

// New function for buffered trace recording
// void
// record_buffered_trace(int pid, const char *cmd_name, const char *sys_name, int retval, int is_exit)
// {
//     acquire(&bufferlock);
    
//     // If this is a new command, reset the buffer
//     if (current_command_pid != pid) {
//         buffer_index = 0;
//         current_command_pid = pid;
//     }

//     // Only record if we have space
//     if (buffer_index < BUFFER_SIZE) {
//         struct buffered_trace *trace = &buffered_events[buffer_index];
//         trace->pid = pid;
//         safestrcpy(trace->command_name, cmd_name, sizeof(trace->command_name));
//         safestrcpy(trace->syscall_name, sys_name, sizeof(trace->syscall_name));
//         trace->retval = retval;
//         trace->is_command_end = is_exit;
//         buffer_index++;
//     }

//     // If this is an exit syscall, print all buffered traces
//     if (is_exit) {
//         cprintf("\n--- System Call Traces for PID %d (%s) ---\n", pid, cmd_name);
//         for (int i = 0; i < buffer_index; i++) {
//             struct buffered_trace *t = &buffered_events[i];
//             if (t->retval == -2) {
//                 cprintf("TRACE: pid=%d | command_name=%s | syscall=%s\n",
//                     t->pid, t->command_name, t->syscall_name);
//             } else {
//                 cprintf("TRACE: pid=%d | command_name=%s | syscall=%s | return value=%d\n",
//                     t->pid, t->command_name, t->syscall_name, t->retval);
//             }
//         }
//         cprintf("----------------------------------------\n");
//         buffer_index = 0;  // Reset buffer
//         current_command_pid = 0;
//     }
    
//     release(&bufferlock);
// }


// void
// record_buffered_trace(int pid, int ppid, const char *cmd_name, const char *sys_name, int retval, int is_exit)
// {
//     acquire(&bufferlock);
    
//     if (buffer_index < BUFFER_SIZE) {
//         struct buffered_trace *trace = &buffered_events[buffer_index];
//         trace->pid = pid;
//         trace->ppid = ppid;
//         safestrcpy(trace->command_name, cmd_name, sizeof(trace->command_name));
//         safestrcpy(trace->syscall_name, sys_name, sizeof(trace->syscall_name));
//         trace->retval = retval;
//         trace->is_command_end = is_exit;
//         trace->trace_index = trace_sequence++;
//         buffer_index++;
//     }

//     // If this is an exit syscall, print all related traces
//     if (is_exit) {
//         cprintf("\n--- System Call Traces ---\n");
        
//         // Print all traces in sequence
//         for (int i = 0; i < buffer_index; i++) {
//             struct buffered_trace *t = &buffered_events[i];
            
//             // Print traces for this process and its parent
//             if (t->pid == pid || t->pid == ppid) {
//                 if (t->retval == -2) {
//                     cprintf("TRACE: pid=%d | command_name=%s | syscall=%s\n",
//                         t->pid, t->command_name, t->syscall_name);
//                 } else {
//                     cprintf("TRACE: pid=%d | command_name=%s | syscall=%s | return value=%d\n",
//                         t->pid, t->command_name, t->syscall_name, t->retval);
//                 }
//             }
//         }
        
//         // Only clear buffer on parent exit or wait
//         if (ppid == 0 || strncmp(sys_name, "wait", 4) == 0) {
//             cprintf("----------------------------------------\n");
//             buffer_index = 0;
//             trace_sequence = 0;
//         }
//     }
    
//     release(&bufferlock);
// }


void
record_buffered_trace(int pid, int ppid, const char *cmd_name, const char *sys_name, int retval, int is_exit)
{
    acquire(&bufferlock);
    
    if (buffer_index < BUFFER_SIZE) {
        struct buffered_trace *trace = &buffered_events[buffer_index];
        trace->pid = pid;
        trace->ppid = ppid;
        safestrcpy(trace->command_name, cmd_name, sizeof(trace->command_name));
        safestrcpy(trace->syscall_name, sys_name, sizeof(trace->syscall_name));
        trace->retval = retval;
        trace->trace_index = trace_sequence++;
        buffer_index++;
    }

    // Print traces when a monitored process exits and it has a parent
    // This ensures we print when child processes finish but not when the shell exits
    if (is_exit && ppid != 0) {
        cprintf("\n--- System Call Traces ---\n");
        
        // Print all buffered traces
        for (int i = 0; i < buffer_index; i++) {
            struct buffered_trace *t = &buffered_events[i];
            // Only print traces for this process and its parent
            if (t->pid == pid || t->pid == ppid) {
                if (t->retval == -2) {
                    cprintf("TRACE: pid=%d | command_name=%s | syscall=%s\n",
                        t->pid, t->command_name, t->syscall_name);
                } else {
                    cprintf("TRACE: pid=%d | command_name=%s | syscall=%s | return value=%d\n",
                        t->pid, t->command_name, t->syscall_name, t->retval);
                }
            }
        }
        
        // Reset buffer
        buffer_index = 0;
        trace_sequence = 0;
    }
    
    // Also print when parent process exits with traces
    if (is_exit && ppid == 0 && buffer_index > 0) {
        cprintf("\n--- System Call Traces ---\n");
        for (int i = 0; i < buffer_index; i++) {
            struct buffered_trace *t = &buffered_events[i];
            if (t->retval == -2) {
                cprintf("TRACE: pid=%d | command_name=%s | syscall=%s\n",
                    t->pid, t->command_name, t->syscall_name);
            } else {
                cprintf("TRACE: pid=%d | command_name=%s | syscall=%s | return value=%d\n",
                    t->pid, t->command_name, t->syscall_name, t->retval);
            }
        }
        buffer_index = 0;
        trace_sequence = 0;
    }
    
    release(&bufferlock);
}

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

void
record_all_traces(int pid, int ppid, const char *cmd_name, const char *sys_name, int retval)
{
    int is_exit = is_exit_syscall(sys_name);
    record_trace_event(pid, cmd_name, sys_name, retval);
    record_buffered_trace(pid, ppid, cmd_name, sys_name, retval, is_exit);
}

