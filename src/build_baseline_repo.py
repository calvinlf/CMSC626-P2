import subprocess

LD_PRELOAD_PATH = "./monitor.so"

# list of commands to build our repo with
COMMANDS = [
    "cat /etc/*",
    "cat /etc/passwd",
    "cat /proc/cpuinfo",
    "ls -l /etc",
    "ls -a /home",
    "cp /etc/passwd /tmp/test_passwd",
    "echo 'hello world' > /tmp/testfile.txt",
    "echo 'hello again world' >> /tmp/testfile.txt",
    "rm /tmp/testfile.txt",
    "bash -c 'echo forked process'",
    "/bin/ls",
    "curl http://example.com",
    "ping -c 2 google.com",
    "wget http://example.com/index.html",
    "printenv",
    "grep 'root' /etc/passwd",
    "wc -l /etc/passwd",
    "gzip -c /etc/passwd > /tmp/passwd.gz",
    "gunzip -c /tmp/passwd.gz > /tmp/passwd",
    "find /etc -name passwd",
    "mv /tmp/passwd /tmp/test_passwd_moved",
    "diff /etc/passwd /tmp/test_passwd_moved"
]

def execute_commands():
    for command in COMMANDS:
        print(f"executing: {command}")
        subprocess.run(["bash", "-c", f"LD_PRELOAD={LD_PRELOAD_PATH} {command}"], check=False)

if __name__ == "__main__":
    execute_commands()
    print("baseline repo built successfully")