FROM gcc:latest

COPY . .

RUN gcc -fPIC -shared forkOverwrite.c -o forkOverwrite.so

RUN gcc -o syscall_trap syscall_trap.c

CMD LD_PRELOAD=./forkOverwrite.so ./syscall_trap