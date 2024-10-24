import socket
import json
import argparse
import sys

# Define constants
BUFFER_SIZE = 1024

def send_json(sock, message):
    """
    Serialize and send a JSON message through the socket.
    """
    try:
        json_data = json.dumps(message)
        sock.sendall(json_data.encode('utf-8'))
        print(f"Sent: {json_data}")
    except Exception as e:
        print(f"Failed to send JSON message: {e}")
        sock.close()
        sys.exit(1)

def receive_json(sock):
    """
    Receive and deserialize a JSON message from the socket.
    """
    try:
        data = sock.recv(BUFFER_SIZE)
        if not data:
            print("No data received from server.")
            sock.close()
            sys.exit(1)
        message = data.decode('utf-8')
        print(f"Received: {message}")
        return json.loads(message)
    except socket.timeout:
        print("Receive timed out.")
        sock.close()
        sys.exit(1)
    except json.JSONDecodeError:
        print("Failed to decode JSON.")
        sock.close()
        sys.exit(1)
    except Exception as e:
        print(f"Error receiving data: {e}")
        sock.close()
        sys.exit(1)

def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="TCP Client for JSON-based communication.")
    parser.add_argument("--ssid", default="netcfg_demo", help="Username for authentication.")
    parser.add_argument("--password", default="12345678", help="Password for authentication.")
    parser.add_argument("--server-ip", default="10.23.149.5", help="Server IP address (default: 10.23.149.5)")
    parser.add_argument("--server-port", type=int, default=29527, help="Server port number (default: 29537)")
    args = parser.parse_args()

    SERVER_IP = args.server_ip
    SERVER_PORT = args.server_port
    USERNAME = args.ssid
    PASSWORD = args.password

    # Create a TCP socket
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            # Connect to the server
            sock.connect((SERVER_IP, SERVER_PORT))
            print(f"Connected to server at {SERVER_IP}:{SERVER_PORT}")

            # Set a timeout for receiving data
            sock.settimeout(5)  # 5 seconds

            # Stage 1: Start Exchange
            stage1 = {"op": "start exchange"}
            send_json(sock, stage1)
            response1 = receive_json(sock)
            print("Stage 1: Server is ready.")
            #if response1.get("status") == "ready":
            #    print("Stage 1: Server is ready.")
            #else:
            #    print("Stage 1: Unexpected server response.")
            #    return

            # Stage 2: Content Exchange with dynamic credentials
            stage2 = {
                "content": "cert",
                "ssid": USERNAME,
                "passwd": PASSWORD
            }
            send_json(sock, stage2)
            response2 = receive_json(sock)
            print("Stage 2: Server is ready.")
            #if response2.get("status") == "received":
            #    print("Stage 2: Server has received the content.")
            #else:
            #    print("Stage 2: Unexpected server response.")
            #    return

            # Stage 3: End Conversation
            stage3 = {"op": "endmsg"}
            send_json(sock, stage3)
            response3 = receive_json(sock)
            print("Stage 3: Server is ready.")
            #if response3.get("status") == "done":
            #    print("Stage 3: Cert exchange is done.")
            #else:
            #    print("Stage 3: Unexpected server response.")
            #    return

            print("Communication with server completed successfully.")

    except socket.error as e:
        print(f"Socket error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
