# HIDS Implementation

To run:

Have Docker running

From the src folder run the following commands:

- `docker build -t gcc .`

- `docker run -it gcc bash`

Test that our shared library was built:

- `ls monitor.so`

Run the test file:

- `LD_PRELOAD=./monitor.so ./targetProcess`

