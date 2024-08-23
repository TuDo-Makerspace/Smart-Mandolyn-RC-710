import socket
import threading

# Define the server addresses and ports
HOST = "192.168.1.132"  # Change this to your specific IP address
PORT1 = 8080  # First port
PORT2 = 8081  # Second port

# TCP command bytes
TCP_ON_DATA = b"\x01"
TCP_OFF_DATA = b"\x00"
TCP_GET_STATE = b"\x03"

# Shared states for both servers
state_8080 = TCP_OFF_DATA
state_8081 = TCP_OFF_DATA


def handle_client(client_socket, client_address, port):
    global state_8080, state_8081

    # print(f"Connected by {client_address} on port {port}")

    # Receive data from the client
    data = client_socket.recv(1024)

    if port == PORT1:
        if data == TCP_GET_STATE:
            # print(f"Port {port}: Received TCP_GET_STATE, sending {state_8080}")
            client_socket.sendall(state_8080)
        elif data == TCP_ON_DATA:
            state_8080 = TCP_ON_DATA
            print(f"Port {port}: Received TCP_ON_DATA, state updated to ON")
        elif data == TCP_OFF_DATA:
            state_8080 = TCP_OFF_DATA
            print(f"Port {port}: Received TCP_OFF_DATA, state updated to OFF")

    elif port == PORT2:
        if data == TCP_GET_STATE:
            # print(f"Port {port}: Received TCP_GET_STATE, sending {state_8081}")
            client_socket.sendall(state_8081)
        elif data == TCP_ON_DATA:
            state_8081 = TCP_ON_DATA
            print(f"Port {port}: Received TCP_ON_DATA, state updated to ON")
        elif data == TCP_OFF_DATA:
            state_8081 = TCP_OFF_DATA
            print(f"Port {port}: Received TCP_OFF_DATA, state updated to OFF")
    else:
        print(f"Port {port}: Received {data}, no response sent")

    # Close the connection
    client_socket.close()


def start_tcp_server(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        # Bind the socket to the address and port
        server_socket.bind((HOST, port))

        # Start listening for incoming connections
        server_socket.listen()
        print(f"Server listening on {HOST}:{port}")

        while True:
            # Accept a connection from a client
            client_socket, client_address = server_socket.accept()
            with client_socket:
                handle_client(client_socket, client_address, port)


if __name__ == "__main__":
    # Start two threads for the two servers
    threading.Thread(target=start_tcp_server, args=(PORT1,), daemon=True).start()
    threading.Thread(target=start_tcp_server, args=(PORT2,), daemon=True).start()

    # Keep the main thread alive to keep the servers running
    print("Servers are running. Press Ctrl+C to stop.")
    while True:
        pass
