#define _GNU_SOURCE       // Needed for RTLD_NEXT
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>       // fork
#include <dlfcn.h>        // dlsym
#include <sys/stat.h>     // open
#include <fcntl.h>        // open

// debug output colors
#define GREEN "\033[0;32m"
#define NC "\033[0m"
#define BLUE "\033[0;34m"

// function pointers to the real functions
static pid_t (*real_fork)(void) = NULL;
static int (*real_open)(const char *pathname, int flags, ...) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;


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
}

// monitoring hooks
pid_t fork(void) {
    printf(" [%s+%s] fork() intercepted\n", BLUE, NC);
    pid_t temp = real_fork();
    return temp;
}

int open(const char *pathname, int flags, ...) {
    printf(" [%s+%s] open() intercepted\n", BLUE, NC);
    int temp = real_open(pathname, flags);
    return temp;
}

ssize_t read(int fd, void *buf, size_t count) {
    printf(" [%s+%s] read() intercepted\n", BLUE, NC);
    ssize_t bytes_read = real_read(fd, buf, count);
    return bytes_read;
}

ssize_t write(int fd, const void *buf, size_t count) {
    printf(" [%s+%s] write() intercepted\n", BLUE, NC);
    ssize_t bytes_written = real_write(fd, buf, count);
    return bytes_written;
}