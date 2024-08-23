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


if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(
        description="TCP client that sends 0x00 and 0x01 repeatedly."
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
        required=True,
        help="Time interval between sending packets (in seconds).",
    )

    args = parser.parse_args()

    # Start sending TCP packets
    send_tcp_packets(args.ip, args.port, args.interval)
