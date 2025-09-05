#!/usr/bin/env python3
"""
Test client for LogCaster-C MVP2
Tests thread pool functionality and query interface
"""

import socket
import sys
import time
import threading

def send_logs(client_id, host='localhost', port=9999, count=10):
    """Connect to server and send test logs"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((host, port))
            # print(f"Client {client_id}: Connected to log server")
            for i in range(count):
                message = f"[Client {client_id}] Log message {i+1} - Thread pool test\n"
                sock.sendall(message.encode())
                time.sleep(0.05)
            # print(f"Client {client_id}: Sent {count} messages")
    except Exception as e:
        print(f"Client {client_id}: Error - {e}")

def query_server(command, host='localhost', port=9998):
    """Send query to server and display response"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((host, port))
            sock.sendall((command + '\n').encode())
            response = sock.recv(4096).decode()
            return response
    except Exception as e:
        return f"Query error: {e}"

def test_thread_pool():
    print("=== Test 1: Thread Pool Concurrency ===")
    threads = []
    client_count = 10
    for i in range(client_count):
        thread = threading.Thread(target=send_logs, args=(i+1, 'localhost', 9999, 5))
        threads.append(thread)
        thread.start()
    for thread in threads:
        thread.join()
    print(f"All {client_count} clients completed sending logs.")
    time.sleep(1)

def test_query_interface():
    print("\n=== Test 2: Query Interface ===")
    print("Querying stats...")
    response = query_server("STATS")
    print(f"Response: {response.strip()}")
    print("\nQuerying count...")
    response = query_server("COUNT")
    print(f"Response: {response.strip()}")
    print("\nSearching for 'message 3'...")
    response = query_server("QUERY keyword=message 3")
    print(f"Response: {response.strip()}")

if __name__ == "__main__":
    print("LogCaster-C MVP2 Test Suite")
    test_thread_pool()
    test_query_interface()
    print("\nAll tests complete.")
