#define _GNU_SOURCE       // Needed for RTLD_NEXT
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       // fork
#include <dlfcn.h>        // dlsym
#include <sys/stat.h>     // open
#include <fcntl.h>        // open
#include <time.h>         // timestamp
#include <string.h>
#include <execinfo.h>     // backtrace
#include <curl/curl.h>

// debug output colors
#define GREEN "\033[0;32m"
#define NC "\033[0m"
#define BLUE "\033[0;34m"

// what database to send to - baseline or monitoring
// the idea is that we set it to baseline upon system initialization
// then after populating the database, switch to monitoring mode and recompile.
#define MONITOR_MODE "baseline"

// flag to indicate if we are in monitoring mode. prevents infinite recursions
static int in_monitoring = 0;

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

// https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
void get_call_stack_sequence (char *stack_buffer, size_t buffer_size){
    void *array[10];
    char **strings;
    int size, i;
    int start_collecting = 0; // dont collect until monitor.so calls are skipped

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);
    if (strings != NULL){
        //printf ("Obtained %d stack frames.\n", size);
        stack_buffer[0] = '\0';
        for (i = 0; i < size; i++) {
            if (strstr(strings[i], "monitor.so") != NULL) {
                start_collecting = 1; // collect only after skipping monitor.so calls
                continue;
            }

            if (start_collecting) {
                size_t len = strnlen(strings[i], buffer_size - 1);
                strncat(stack_buffer, strings[i], buffer_size - 1);
                strncat(stack_buffer, "; ", buffer_size - 1);
                buffer_size -= len + 2;
            }
        }
    }

    free(strings);
}

// utility function to send json payload to backend
// https://curl.se/libcurl/c/http-post.html
void send_to_backend(const char *url, const char *json_payload){
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

// utility function to log syscall to database
// we care about the sequence of syscalls per process;
// this way we can detect if a specific process is making abnormal syscalls
void log_syscall(char *syscall_name){
    pid_t pid = getpid();
    time_t timestamp = get_timestamp();
    
    char process_name[256];
    get_process_name(pid, process_name);

    char call_stack[2048];
    get_call_stack_sequence(call_stack, sizeof(call_stack));

    printf("[%ld] PID: %d, Process: %s, Syscall: %s\n", timestamp, pid, process_name, syscall_name);
    printf("Call Stack: %s\n", call_stack);
    
    // send json payload to backend
    // backend will insert into db as well as analyze for anomalies
    char json_payload[1024];
    snprintf(json_payload, sizeof(json_payload), "{\"timestamp\": %ld, \"pid\": %d, \"process\": \"%s\", \"syscall\": \"%s\", \"call_stack\": \"%s\"}", timestamp, pid, process_name, syscall_name, call_stack);
    const char *url = (strcmp(MONITOR_MODE, "baseline") == 0) ? "http://host.docker.internal:8080/baseline-log" : "http://host.docker.internal:8080/monitor-log";
    send_to_backend(url, json_payload);
}

// monitoring hooks
pid_t fork(void) {
    log_syscall("fork");
    pid_t temp = real_fork();
    return temp;
}

int open(const char *pathname, int flags, ...) {
    if (in_monitoring){
        return real_open(pathname, flags);
    }

    in_monitoring = 1;
    log_syscall("open");
    int temp = real_open(pathname, flags);
    in_monitoring = 0;
    return temp;
}

ssize_t read(int fd, void *buf, size_t count) {
    if (in_monitoring){
        return real_read(fd, buf, count);
    }

    in_monitoring = 1;
    log_syscall("read");
    ssize_t bytes_read = real_read(fd, buf, count);
    in_monitoring = 0;
    return bytes_read;
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (in_monitoring){
        return real_write(fd, buf, count);
    }

    in_monitoring = 1;
    log_syscall("write");
    ssize_t bytes_written = real_write(fd, buf, count);
    in_monitoring = 0;
    return bytes_written;
}

int close(int fd) {
    if(in_monitoring){
        return real_close(fd);
    }

    in_monitoring = 1;
    log_syscall("close");
    int temp = real_close(fd);
    in_monitoring = 0;
    return temp;
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    log_syscall("execve");
    int temp = real_execve(filename, argv, envp);
    return temp;
}