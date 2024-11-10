#define _GNU_SOURCE       // Needed for RTLD_NEXT
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>       // fork
#include <dlfcn.h>        // dlsym


static int (*real_fork)(void) = NULL;

/////////////////////////////////////////////////////////
// Use a constructor to run at load time to look up the
// real address for fork().  This enables us to use
// the real fork() function from our hijacked function.
/////////////////////////////////////////////////////////
void __attribute__((constructor)) backdoor_initalize() {
  real_fork = dlsym(RTLD_NEXT, "fork");
  #ifdef DEBUG
  printf(" [%s+%s] %sBACKDOOR: fork() Backdoor Loaded!\n%s",
         GREEN, NC, GREEN, NC);
  printf(" [%s+%s] %sBACKDOOR: Real fork() addr: %s%p%s\n",
         GREEN, NC, GREEN, BLUE, real_fork, NC);
  #endif

}

// Our version of fork
pid_t fork(void) {
    printf("Here\n");
    pid_t temp = real_fork();
    return temp;
}