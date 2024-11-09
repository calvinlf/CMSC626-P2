#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>

int main() {
    // create child process
    pid_t child;
    long orig_rax;

    child = fork();

    // child process
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    // parent process
    else {
        wait(NULL);

        // get syscall number of child process
        orig_rax = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * ORIG_RAX, NULL);
        printf("child made syscall %ld\n", orig_rax);

        // example - trap execve. do something in here once we trap it
        if (orig_rax == SYS_execve)
            printf("trapped execve syscall\n");
    }

    return 0;
}
