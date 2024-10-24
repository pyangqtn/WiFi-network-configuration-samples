import socket
import json

SERVER_IP = "10.23.149.5"  #Replace with your server's IP address
SERVER_PORT = 29527
CLIENT_PORT = 29528
BUFFER_SIZE = 1024

def send_json(sock, message):
    json_data = json.dumps(message)
    sock.sendto(json_data.encode('utf-8'), (SERVER_IP, SERVER_PORT))
    print(f"Sent: {json_data}")

def receive_json(sock):
    data, addr = sock.recvfrom(BUFFER_SIZE)
    message = data.decode('utf-8')
    print(f"Received: {message}")
    return json.loads(message)

def main():
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Stage 1: Start exchange
    start_message = {"op": "start exchange"}
    send_json(sock, start_message)
    response = receive_json(sock)
    if response.get("status") != "ready":
        print("Error: Unexpected server response")
        return

    # Stage 2: Send credentials
    credentials_message = {
        "content": "cert",
        "username": "text1",
        "password": "text2"
    }
    send_json(sock, credentials_message)
    response = receive_json(sock)
    if response.get("status") != "received":
        print("Error: Unexpected server response")
        return

    # Stage 3: End conversation
    end_message = {"op": "endconversation"}
    send_json(sock, end_message)
    response = receive_json(sock)
    if response.get("status") != "ready":
        print("Error: Unexpected server response")

    # Close the socket
    sock.close()

if __name__ == "__main__":
    main()

