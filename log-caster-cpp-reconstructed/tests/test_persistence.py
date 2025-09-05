#!/usr/bin/env python3
import os
import socket
import subprocess
import time
import shutil

HOST = '127.0.0.1'
LOG_PORT = 9999
LOG_DIR = "./test_logs"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SERVER_EXEC = os.path.join(SCRIPT_DIR, "../build/logcaster-cpp")

def send_log(message):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        s.sendall((message + '\n').encode())

def cleanup():
    if os.path.exists(LOG_DIR):
        shutil.rmtree(LOG_DIR)

def run_test():
    cleanup()

    # --- Test 1: Persistence disabled ---
    print("--- Test 1: Persistence disabled ---")
    server_proc = subprocess.Popen([SERVER_EXEC])
    time.sleep(1)
    send_log("test_no_persistence")
    server_proc.terminate()
    server_proc.wait()
    assert not os.path.exists(LOG_DIR), "Log directory should not be created when -P is not set"
    print("OK\n")

    # --- Test 2: Persistence enabled ---
    print("--- Test 2: Persistence enabled ---")
    os.makedirs(LOG_DIR)
    server_proc = subprocess.Popen([SERVER_EXEC, "-P", "-d", LOG_DIR])
    time.sleep(1) # Allow time for async write
    log_message = "hello persistence world"
    send_log(log_message)
    time.sleep(1) # Allow time for async write
    server_proc.terminate()
    server_proc.wait()

    log_file = os.path.join(LOG_DIR, "current.log")
    assert os.path.exists(log_file), "current.log should be created"
    with open(log_file, 'r') as f:
        content = f.read()
        assert log_message in content, "Log message not found in file"
    print("OK\n")

    cleanup()

if __name__ == "__main__":
    run_test()
    print("All MVP4 tests passed!")