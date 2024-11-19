FROM gcc:latest

COPY . .

RUN gcc -fPIC -shared monitor.c -o monitor.so

CMD ["bash", "-c", "LD_PRELOAD=./monitor.so"]
