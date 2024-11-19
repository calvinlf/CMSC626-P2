#define _GNU_SOURCE       // Needed for RTLD_NEXT
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>       // fork
#include <dlfcn.h>        // dlsym
#include <sys/stat.h>     // open
#include <fcntl.h>        // open
#include <time.h>         // timestamp
#include <string.h>

// debug output colors
#define GREEN "\033[0;32m"
#define NC "\033[0m"
#define BLUE "\033[0;34m"

// function pointers to the real functions (definitions via manpages)
static pid_t (*real_fork)(void) = NULL;
static int (*real_open)(const char *pathname, int flags, ...) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static int (*real_close)(int fd) = NULL;
static int (*real_execve)(const char *filename, char *const argv[], char *const envp[]) = NULL;


/////////////////////////////////////////////////////////
// Use a constructor to run at load time to look up the
// real addresses for syscall functions.  This enables us to use
// the real functions from our hijacked functions.
/////////////////////////////////////////////////////////
void __attribute__((constructor)) backdoor_initalize() {
    real_fork = dlsym(RTLD_NEXT, "fork");
    real_open = dlsym(RTLD_NEXT, "open");
    real_read = dlsym(RTLD_NEXT, "read");
    real_write = dlsym(RTLD_NEXT, "write");
    real_close = dlsym(RTLD_NEXT, "close");
    real_execve = dlsym(RTLD_NEXT, "execve");
  
    printf(" [%s+%s] %sBACKDOOR: Backdoors Loaded!\n%s",
           GREEN, NC, GREEN, NC);
    printf(" [%s+%s] %sBACKDOOR: Real fork() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_fork, NC);
    printf(" [%s+%s] %sBACKDOOR: Real open() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_open, NC);
    printf(" [%s+%s] %sBACKDOOR: Real read() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_read, NC);
    printf(" [%s+%s] %sBACKDOOR: Real write() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_write, NC);
    printf(" [%s+%s] %sBACKDOOR: Real close() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_close, NC);
    printf(" [%s+%s] %sBACKDOOR: Real execve() addr: %s%p%s\n",
           GREEN, NC, GREEN, BLUE, real_execve, NC);
}

// utility function to get the current timestamp
time_t get_timestamp(){
    return time(NULL);
}

// utility function to get process name by pid
// https://gist.github.com/fclairamb/a16a4237c46440bdb172
static void get_process_name(const pid_t pid, char * name) {
    char procfile[256];
    sprintf(procfile, "/proc/%d/cmdline", pid);
    FILE* f = fopen(procfile, "r");
    if (f) {
       size_t size;
       size = fread(name, sizeof (char), sizeof (procfile), f);
       if (size > 0) {
           if ('\n' == name[size - 1])
           name[size - 1] = '\0';
       }
       fclose(f);
    }
}

// utility function to log syscall to database
// we care about the sequence of syscalls per process;
// this way we can detect if a specific process is making abnormal syscalls
void log_syscall(char *syscall_name){
    pid_t pid = getpid();
    time_t timestamp = get_timestamp();
    
    char process_name[256];
    get_process_name(pid, process_name);

    printf("[%ld] PID: %d, Process: %s, Syscall: %s\n", timestamp, pid, process_name, syscall_name);
    // to-do send to db
    // need a mechanism for determining whether to send to baseline db or monitoring db.
}

// monitoring hooks
pid_t fork(void) {
    log_syscall("fork");
    pid_t temp = real_fork();
    return temp;
}

int open(const char *pathname, int flags, ...) {
    log_syscall("open");
    int temp = real_open(pathname, flags);
    return temp;
}

ssize_t read(int fd, void *buf, size_t count) {
    log_syscall("read");
    ssize_t bytes_read = real_read(fd, buf, count);
    return bytes_read;
}

ssize_t write(int fd, const void *buf, size_t count) {
    log_syscall("write");
    ssize_t bytes_written = real_write(fd, buf, count);
    return bytes_written;
}

int close(int fd) {
    log_syscall("close");
    int temp = real_close(fd);
    return temp;
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    log_syscall("execve");
    int temp = real_execve(filename, argv, envp);
    return temp;
}