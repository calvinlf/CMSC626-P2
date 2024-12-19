#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;

// example infection - simulate a process arbitrarily making fork and execve calls
// just hooks read syscall and if it reads anything, it executes /usr/bin/echo in a forked process
ssize_t read(int fd, void *buf, size_t count) {
    real_read = dlsym(RTLD_NEXT, "read");
    ssize_t bytes_read = real_read(fd, buf, count);

    if (bytes_read > 0) {
        const char *path = "/usr/bin/echo";
        char *const argv[] = {(char *)path, "infected!!!", NULL};
        char *const envp[] = {NULL};

        if (fork() == 0) {
            execve(path, argv, envp);
            perror("execve failed");
            _exit(EXIT_FAILURE);
        }
    }

    return bytes_read;
}

