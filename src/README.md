﻿# HIDS Implementation

To run:
Have Docker running
From the src folder run the following commands:
- `docker build -t gcc .`
- `docker run -it gcc bash`

Test that our shared library was built:
- `ls monitor.so`

Test the shared library on a process (example: cat etc/passwd):
- `LD_PRELOAD=./monitor.so cat etc/passwd`

To run backend:
On your local machine, make sure you have flask
- `pip install flask`

Start the backend server on your local machine
- `python3 backend.py`

In your docker container, you can verify the connection:
- `curl host.docker.internal:8080` (or whichever port server is running on)

Install mongodb, ensure it's running on localhost:27017
