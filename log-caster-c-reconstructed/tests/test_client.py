# [SEQUENCE: MVP1-31]
import socket
import threading
import time
import sys

SERVER_HOST = '127.0.0.1'
SERVER_PORT = 9999
NUM_CLIENTS_MULTIPLE = 5
NUM_CLIENTS_STRESS = 50
MESSAGES_PER_CLIENT = 10

def client_task(client_id, num_messages):
    """A single client connects, sends messages, and closes."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_HOST, SERVER_PORT))
            print(f"Client {client_id}: Connected to server.")
            for i in range(num_messages):
                message = f"Client {client_id}, message {i+1}\n"
                s.sendall(message.encode('utf-8'))
                # print(f"Client {client_id}: Sent '{message.strip()}'")
                time.sleep(0.1)
            print(f"Client {client_id}: All messages sent.")
    except ConnectionRefusedError:
        print(f"Client {client_id}: Connection refused. Is the server running?")
    except Exception as e:
        print(f"Client {client_id}: An error occurred: {e}")
    finally:
        print(f"Client {client_id}: Connection closed.")

def run_test(num_clients, num_messages):
    """Runs a test with a specified number of clients."""
    threads = []
    start_time = time.time()

    print(f"--- Starting test with {num_clients} clients ---")
    for i in range(num_clients):
        thread = threading.Thread(target=client_task, args=(i + 1, num_messages))
        threads.append(thread)
        thread.start()
        time.sleep(0.05) # Stagger connections slightly

    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"\nTest with {num_clients} clients finished in {end_time - start_time:.2f} seconds.")

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in ['single', 'multiple', 'stress']:
        print("Usage: python test_client.py [single|multiple|stress]")
        sys.exit(1)

    mode = sys.argv[1]

    if mode == 'single':
        run_test(1, MESSAGES_PER_CLIENT)
    elif mode == 'multiple':
        run_test(NUM_CLIENTS_MULTIPLE, MESSAGES_PER_CLIENT)
    elif mode == 'stress':
        run_test(NUM_CLIENTS_STRESS, 5) # Fewer messages for stress test
