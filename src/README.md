# HIDS Implementation

To run:
Have Docker running
From the src folder run the following commands:
- `docker build -t gcc .`
- `docker run -it gcc bash`

Test that our shared library was built:
- `ls monitor.so`

Install mongodb on local machine, ensure it's running on localhost:27017
On your local machine, make sure you have flask
- `pip install flask`

Start the backend server on your local machine
- `python3 backend.py`

In your docker container, you can verify the connection:
- `curl host.docker.internal:8080` (or whichever port server is running on)

Build the baseline repository
- `python3 build_baseline_repo.py`

Root user/admin can set monitoring mode using environment vars:
- `export MONITOR_MODE=monitoring` or `export MONITOR_MODE=baseline` (defaults to baseline)

Run a performance evaluation
- `python3 performance_evaluation.py`

Test run for the shared library on a process (example: cat etc/passwd):
- `LD_PRELOAD=./monitor.so cat etc/passwd`

Simulate infecting processes that make "read" syscalls (should be run only in monitoring mode):
- `LD_PRELOAD="./monitor.so ./infect.so" cat /etc/passwd`

Set LD_PRELOAD envvar to have it persist without needing to set it per process
- `export LD_PRELOAD=./monitor.so`
