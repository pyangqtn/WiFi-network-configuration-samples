import socket
import json

# Server details
SERVER_IP = '10.23.149.5'  # Replace with the actual IP address of the server
SERVER_PORT = 29527           # The port the server is listening on

# JSON data to send
data = {
    "name": "FreeRTOS Client",
    "message": "Hello from Python!"
}

# Convert the dictionary to a JSON string
json_data = json.dumps(data)

def udp_client():
    # Create a TCP/IP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        # Connect the socket to the server
        client_socket.connect((SERVER_IP, SERVER_PORT))
        print(f"Connected to server at {SERVER_IP}:{SERVER_PORT}")

        # Send the JSON data
        client_socket.sendto(json_data.encode('utf-8'), (SERVER_IP, SERVER_PORT))
        print(f"Sent JSON data: {json_data}")

        # Optionally, receive a response from the server
#        response = client_socket.recv(1024).decode('utf-8')
#        print(f"Received from server: {response}")

    except Exception as e:
        print(f"Error: {e}")

    finally:
        # Close the socket
        client_socket.close()
        print("Connection closed.")

if __name__ == '__main__':
    udp_client()
