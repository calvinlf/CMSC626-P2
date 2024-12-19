from flask import Flask, request, jsonify
from pymongo import MongoClient
from difflib import SequenceMatcher

PORT = 8080

app = Flask(__name__)

# testing route
@app.route('/', methods=["GET"])
def home():
    return f"backend server running on port {PORT}\n"

# mongodb connection
client = MongoClient("mongodb://localhost:27017/")
db = client["syscall_logs"]
baseline_collection = db["baseline_logs"]
monitoring_collection = db["monitoring_logs"]

# receive baseline log from monitor.c, post to DB
@app.route('/baseline-log', methods=["POST"])
def receive_baseline_log():
    data = request.json
    res = baseline_collection.insert_one(data)
    return jsonify({"status": "success", "message": "baseline log received", "id": str(res.inserted_id)})

# receive monitoring log from monitor.c, check for anomaly + post to DB
@app.route('/monitor-log', methods=["POST"])
def receive_monitor_log():
    data = request.json
    res = monitoring_collection.insert_one(data)

    # check for anomaly
    if anomaly_detected(data):
        return jsonify({"status": "anomaly_detected", "message": "anomaly detected", "id": str(res.inserted_id)})

    return jsonify({"status": "success", "message": "monitoring log received", "id": str(res.inserted_id)})

# similarity-based anomaly detection
def anomaly_detected(log):
    # these params should be adjustable for the admin user to control how strict/relaxed they want anomaly detection to be
    similarity_threshold = 0.5  # how similar current log syscall sequence is to avg baseline sequence - 0.5 is somewhat lenient
    new_syscall_threshold = 0.1  # proportion of allowable new syscalls, i set it to be more strict

    # get matching logs from baseline for the same process
    baseline_logs = list(baseline_collection.find({"process": log["process"]}))

    # only look at ones with unique (PID, process name)
    syscall_sequences = {}
    for entry in baseline_logs:
        pid = entry.get("pid")
        syscall = entry.get("syscall")
        if pid not in syscall_sequences:
            syscall_sequences[pid] = []
        syscall_sequences[pid].append(syscall)

    # get the current log's syscall sequence
    current_pid = log["pid"]
    current_syscalls = list(monitoring_collection.find({"process": log["process"], "pid": current_pid}))
    current_sequence = [entry["syscall"] for entry in current_syscalls]

    # compare the current log syscall sequence to all baseline log sequences
    # things to consider: 
    # similarity of syscall sequences in general
    # are there any new syscalls in the current log not seen in the baseline?
    # if length of current sequence is less than length of baseline sequence, only compare the first n syscalls up to length of current sequence
    # (and other way around if current sequence is longer)
    total_similarity = 0
    similarity_count = 0
    all_new_syscalls = set()
    for pid, baseline_sequence in syscall_sequences.items():
        # truncate the sequences to the same length for comparison
        compare_length = min(len(current_sequence), len(baseline_sequence))
        truncated_baseline = baseline_sequence[:compare_length]
        truncated_current = current_sequence[:compare_length]

        # compute similarity for the truncated sequences
        similarity = SequenceMatcher(None, truncated_current, truncated_baseline).ratio()
        total_similarity += similarity
        similarity_count += 1

        # check for new syscalls
        new_syscalls = set(current_sequence) - set(baseline_sequence)
        all_new_syscalls.update(new_syscalls)

    # avg similarity of all baseline logs compared to current log, and proportion of new syscalls in current log
    average_similarity = total_similarity / similarity_count
    total_new_syscalls = len(all_new_syscalls)
    new_syscall_proportion = total_new_syscalls / len(current_sequence)

    if average_similarity < similarity_threshold or new_syscall_proportion > new_syscall_threshold:
        print(f"\033[91manomaly detected for PID {current_pid} ({log["process"]})\033[0m")
        print(f"\033[91maverage similarity: {average_similarity:.2f}\033[0m")
        print(f"\033[91mnew syscall proportion: {new_syscall_proportion:.2f}\033[0m")
        print(f"\033[91mnew syscalls: {list(all_new_syscalls)}\033[0m")
        return True

    print(f"no anomalies detected for PID {current_pid} (average similarity: {average_similarity:.2f}, new syscall proportion: {new_syscall_proportion:.2f})")
    return False

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=PORT)