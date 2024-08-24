import socket
import time
import argparse


def send_tcp_packets(ip, port, interval):
    try:
        # Create a TCP/IP socket
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            # Connect to the server
            sock.connect((ip, port))
            print(f"Connected to {ip}:{port}")

            while True:
                # Send 0x00
                sock.sendall(b"\x00")
                print("Sent: 0x00")
                time.sleep(interval)

                # Send 0x01
                sock.sendall(b"\x01")
                print("Sent: 0x01")
                time.sleep(interval)

    except Exception as e:
        print(f"An error occurred: {e}")


def send_single_tcp_packet(ip, port, data):
    try:
        # Create a TCP/IP socket
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            # Connect to the server
            sock.connect((ip, port))
            print(f"Connected to {ip}:{port}")

            # Send the specified data
            sock.sendall(data)
            print(f"Sent: {data.hex()}")

            # If the command is "get", wait for a response
            if data == b"\x03":
                response = sock.recv(1024)
                print(f"Received: {response.hex()}")

            time.sleep(0.1)  # Give time for ACK

    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(
        description="TCP client that sends 0x00 and 0x01 repeatedly or specific commands."
    )
    parser.add_argument(
        "--ip", type=str, required=True, help="The IP address of the TCP server."
    )
    parser.add_argument(
        "--port", type=int, required=True, help="The port number of the TCP server."
    )
    parser.add_argument(
        "--interval",
        type=float,
        help="Time interval between sending 0x00 and 0x01 packets (in seconds).",
    )
    parser.add_argument(
        "--on", action="store_true", help="Send 0x01 to turn on the device."
    )
    parser.add_argument(
        "--off", action="store_true", help="Send 0x00 to turn off the device."
    )
    parser.add_argument(
        "--get", action="store_true", help="Send 0x03 to get the current state."
    )

    args = parser.parse_args()

    # Check if interval is set, otherwise use single command
    if args.interval is not None:
        send_tcp_packets(args.ip, args.port, args.interval)
    elif args.on:
        send_single_tcp_packet(args.ip, args.port, b"\x01")
    elif args.off:
        send_single_tcp_packet(args.ip, args.port, b"\x00")
    elif args.get:
        send_single_tcp_packet(args.ip, args.port, b"\x03")
    else:
        print("Please specify --interval, --on, --off, or --get.")
