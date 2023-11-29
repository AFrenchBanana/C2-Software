#!/usr/bin/env python3
import socket
import struct

def send_data(conn, data):
    """function that sends data across a socket,
    socket connection and data gets fed in, the length of data is then calculated.
    the socket sends a header file packed with struct that sends the total length and chunk size
    The data is sent in chunks of 4096 until the last chunk where only the required amount is sent.
    the function allows for raw bytes and strings to be sent for multiple data types to be sent.
    """
    total_length = len(data) # calculates the total length
    chunk_size = 4096 # sets the chunk size
    print(total_length, chunk_size)
    conn.sendall(struct.pack('!II', total_length, chunk_size))   
    for i in range(0, total_length, chunk_size): # range of total length incrementing in chunksize
        end_index = i + chunk_size if i + chunk_size < total_length else total_length # calculates how much data needs to be sent
        chunk = data[i:end_index] # makes the chunk with the required amount of data
        try:
            conn.sendall(chunk.encode()) #trys to encode the chunks and send them
        except AttributeError:
            conn.sendall(chunk) # if it cant be encoded sends it as it is.
    return

def receive_data(conn):
    """function that recieves data
    first the header is recieved with the chunk size and total length
    after this it recieves data in the chunk size until the last packet where it is the remaining length"""
    try:
        total_length, chunk_size = struct.unpack('!II',conn.recv(8)) #unpacks the header length
        print(total_length, chunk_size)
        received_data = b'' # sets receveid bytes to a bytes string
        while total_length > 0: # loop until total_length is less than 0 
            chunk = conn.recv(min(chunk_size, total_length)) # receives chunk based off whatever is smaller, total length or chunk_size
            received_data += chunk # adds the chunk to recieved data
            total_length -= len(chunk) # removes the chunk length from the total_length
        try:
            received_data = received_data.decode("utf-8") # attempts to decode the data
        except UnicodeDecodeError:
            pass # else doesnt decode the data and returns it as bytes
    except struct.error:
        pass
    print(received_data)
    return received_data



# Define the host and port
host = 'localhost'
port = 11001

# Create a socket object
listener_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the host and port
listener_socket.bind((host, port))

# Listen for incoming connections
listener_socket.listen(1)

print(f"Listening for incoming connections on {host}:{port}...")

# Accept incoming connections
client_socket, client_address = listener_socket.accept()

print(f"Accepted connection from {client_address[0]}:{client_address[1]}")
send_data(client_socket, "12345")
# Receive data from the client
print(receive_data(client_socket))
# Close the client socket and the listener socket
client_socket.close()
listener_socket.close()
