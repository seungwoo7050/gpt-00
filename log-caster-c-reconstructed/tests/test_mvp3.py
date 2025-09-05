#!/usr/bin/env python3
import socket
import time
import threading

HOST = '127.0.0.1'
LOG_PORT = 9999
QUERY_PORT = 9998

def send_logs():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, LOG_PORT))
        logs = [
            "2025-07-28 INFO: Application startup successful",
            "2025-07-28 DEBUG: Database connection pool created with 10 connections",
            "2025-07-28 WARNING: Configuration value 'timeout' is deprecated, use 'connection_timeout' instead",
            "2025-07-28 ERROR: Failed to process user request for user_id=123, reason: timeout",
            "2025-07-28 INFO: User logged in: admin",
            "2025-07-28 ERROR: Critical failure in payment module, transaction_id=xyz-789"
        ]
        for log in logs:
            s.sendall((log + '\n').encode())
            time.sleep(0.1)

def query_server(query):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, QUERY_PORT))
        s.sendall((query + '\n').encode())
        response = s.recv(4096).decode()
        return response

def run_test(description, query, expected_lines):
    print(f"--- {description} ---")
    print(f"> {query}")
    response = query_server(query)
    print(response.strip())
    assert response.startswith(f"FOUND: {expected_lines}")
    print("OK\n")

if __name__ == "__main__":
    # Give server a moment to start
    time.sleep(1)
    # Send some initial logs
    log_thread = threading.Thread(target=send_logs)
    log_thread.start()
    # Wait for logs to be processed
    time.sleep(1)

    # Run tests
    run_test("Test 1: Simple keyword search", "QUERY keywords=timeout", 2)
    run_test("Test 2: Regex search", "QUERY regex=user_id=[0-9]+", 1)
    run_test("Test 3: Multi-keyword AND search", "QUERY keywords=Failed,timeout operator=AND", 1)
    run_test("Test 4: Multi-keyword OR search", "QUERY keywords=startup,failure operator=OR", 2)
    
    # Assuming current time is around 2025-07-28
    # These timestamps may need adjustment depending on when the test is run
    # For simplicity, we search for a wide range
    run_test("Test 5: Time range search", f"QUERY time_from=1753689600 time_to=1753862400", 6)
    run_test("Test 6: Combined search (Regex and Keyword)", "QUERY regex=deprecate keywords=WARNING", 1)
    run_test("Test 7: HELP command", "HELP", 13)

    log_thread.join()
    print("All MVP3 tests passed!")
