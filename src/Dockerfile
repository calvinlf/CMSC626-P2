FROM gcc:latest

COPY . .

RUN gcc -fPIC -shared monitor.c -o monitor.so -lcurl

RUN gcc -fPIC -shared infect.c -o infect.so

ENV MONITOR_MODE=baseline

CMD ["bash", "-c", "LD_PRELOAD=./monitor.so"]
