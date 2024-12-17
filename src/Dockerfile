FROM gcc:latest

COPY . .

RUN gcc -fPIC -shared monitor.c -o monitor.so -lcurl

CMD ["bash", "-c", "LD_PRELOAD=./monitor.so"]
