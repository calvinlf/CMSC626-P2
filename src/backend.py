from flask import Flask, request, jsonify
from pymongo import MongoClient

PORT = 8080

app = Flask(__name__)

# testing route
@app.route('/', methods=["GET"])
def home():
    return "backend server running on port {PORT}\n"

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

# rule-based anomaly detection
def anomaly_detected(log):
    # to-do
    return False

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=PORT)