import subprocess
import time
import statistics

LD_PRELOAD_PATH = "./monitor.so"

COMMANDS = [
    "cat /etc/passwd",
    "ls -l /etc",
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

NUM_ITERATIONS = 10


def measure_time(command, use_ld_preload):
    start_time = time.time()
    with open("/dev/null", "w") as devnull:
        subprocess.run(["bash", "-c", f"LD_PRELOAD={LD_PRELOAD_PATH if use_ld_preload else ''} {command}"], check=False, stdout=devnull, stderr=devnull)

    end_time = time.time()

    return end_time - start_time


def execute_commands():
    results = []

    for command in COMMANDS:
        base_times = []
        monitor_times = []
        print(f"executing: {command}")

        for i in range(NUM_ITERATIONS):
            base_times.append(measure_time(command, use_ld_preload=False))
            monitor_times.append(measure_time(command, use_ld_preload=True))
        
        base_avg = statistics.mean(base_times)
        monitor_avg = statistics.mean(monitor_times)
        slowdown = monitor_avg / base_avg

        results.append({
            "command": command,
            "base_avg": base_avg,
            "monitor_avg": monitor_avg,
            "slowdown": slowdown,
            "base_min": min(base_times),
            "monitor_min": min(monitor_times),
            "base_max": max(base_times),
            "monitor_max": max(monitor_times),
        })

    print("\nperformance evaluation results:")
    print(f"{'command':<50} {'base avg':<10} {'monitor avg':<12} {'slowdown':<10} {'base min':<10} {'monitor min':<12} {'base max':<10} {'monitor max':<12}")
    print("=" * 150)

    total_base_avg = []
    total_monitor_avg = []

    for result in results:
        print(f"{result['command']:<50} "
                f"{result['base_avg']:<10.4f} {result['monitor_avg']:<12.4f} {result['slowdown']:<10.2f} "
                f"{result['base_min']:<10.4f} {result['monitor_min']:<12.4f} {result['base_max']:<10.4f} "
                f"{result['monitor_max']:<12.4f} ")

        total_base_avg.append(result['base_avg'])
        total_monitor_avg.append(result['monitor_avg'])

    print ("=" * 150)
    overall_base_avg = statistics.mean(total_base_avg)
    overall_monitor_avg = statistics.mean(total_monitor_avg)
    overall_slowdown = overall_monitor_avg / overall_base_avg

    print(f"{'overall average':<50} {overall_base_avg:<10.4f} {overall_monitor_avg:<12.4f} {overall_slowdown:<10.2f}\n")

if __name__ == "__main__":
    print(f"\nrunning performance evaluation, {NUM_ITERATIONS} iterations per command...")
    execute_commands()