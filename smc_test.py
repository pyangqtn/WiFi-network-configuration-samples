import socket
import argparse
import time

# Constants
PRE_ASSIGNED_LENGTH = 996
PRE_PAYLOAD_ADD = 64
ETHERNET_HEADER = 14
IP_HEADER = 20
UDP_HEADER = 8
WIFI_HEADER = 24  # Adjust as per your wireless environment
TOTAL_OVERHEAD = ETHERNET_HEADER + IP_HEADER + UDP_HEADER + WIFI_HEADER

# Argument parsing
def parse_args():
    parser = argparse.ArgumentParser(description="Send UDP broadcast with specific packet lengths")
    parser.add_argument("--ssid", type=str, default="smc", help="SSID to use")
    parser.add_argument("--password", type=str, default="netcfg_demo", help="Password to use")
    parser.add_argument("--bind", type=str, help="IP address or interface to bind to")
    parser.add_argument("--port", type=int, default=12345, help="Target port")
    parser.add_argument("--target-ip", type=str, default="255.255.255.255", help="Target IP address for broadcasting")
    parser.add_argument("--interval", type=float, default=0.001, help="Interval between frames in seconds (default: 1 ms)")
    parser.add_argument("--loop", type=int, default=3, help="Number of times to repeat the procedure (default: 3)")
    parser.add_argument("--fixlen", type=int, default=10, help="Differ of pkt to actual wireless frame(default: 10)")
    return parser.parse_args()

# Function to send broadcast frames
def send_broadcast(sock, length, target_ip, target_port, interval, fixlen):
    payload_size = length + PRE_PAYLOAD_ADD - TOTAL_OVERHEAD - fixlen
#    payload_size = length - TOTAL_OVERHEAD
    message = b'a' * payload_size
    sock.sendto(message, (target_ip, target_port))
    time.sleep(interval)  # Adding the interval between frames

# Procedure to send UDP broadcast frames according to the rules
def broadcast_procedure(sock, args):
    # Step 1: Send 3 frames with length 996
    for _ in range(3):
        send_broadcast(sock, PRE_ASSIGNED_LENGTH, args.target_ip, args.port, args.interval, args.fixlen)

    # Step 2: Send frames with ASCII values of SSID
    for char in args.ssid:
        send_broadcast(sock, ord(char), args.target_ip, args.port, args.interval, args.fixlen)
        print(f"Send ssid  {ord(char)}")

    # Step 3: Send 3 frames with length (996 + 8)
    for _ in range(3):
        send_broadcast(sock, PRE_ASSIGNED_LENGTH + 8, args.target_ip, args.port, args.interval, args.fixlen)

    # Step 4: Send frames with ASCII values of password
    for char in args.password:
        send_broadcast(sock, ord(char), args.target_ip, args.port, args.interval, args.fixlen)
        print(f"Send passwd  {ord(char)}")

    # Step 5: Send 3 frames with length (996 + 16)
    for _ in range(3):
        send_broadcast(sock, PRE_ASSIGNED_LENGTH + 16, args.target_ip, args.port, args.interval, args.fixlen)

# Main function
def main():
    args = parse_args()

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    # Bind the socket if --bind is provided
    if args.bind:
        sock.bind((args.bind, 0))

    # Execute the broadcast procedure for "loop" times
    for i in range(args.loop):
        print(f"Executing loop {i+1}/{args.loop}")
        broadcast_procedure(sock, args)

    print("Broadcasts sent successfully!")

if __name__ == "__main__":
    main()

