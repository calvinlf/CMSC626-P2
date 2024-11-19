FROM gcc:latest

# copy source code to docker container
COPY monitor.c .

RUN gcc -fPIC -shared monitor.c -o monitor.so

#RUN gcc -o test_fork test_fork.c

CMD ["bash", "-c", "LD_PRELOAD=./monitor.so"]
