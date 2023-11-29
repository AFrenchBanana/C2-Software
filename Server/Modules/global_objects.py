"""
Global Objects that are allowed to be accessed anywhere within the project. 
This allows for a single form of management with items such as connected sockets 
and the ability alter them.

Contains error handled send and recieve functions that can handle bytes and strings
"""

import struct
from tqdm import tqdm
from Modules.GUI import *
import sys

#global socket details to alllow multiple connections and the ability
#to interact with them individually.
connectionaddress = []
connectiondetails = []
hostname = []

app = QApplication(sys.argv)
window = MainWindow()



def add_connection_list(conn, r_address, host):
    """this function places the connection details into 3 lists, one for each variable upon succesful socket connection
    this allows the connection to be accessed from anywhere within the server"""
    connectiondetails.append(conn) # the socket connection details
    connectionaddress.append(r_address) #the ip address and port
    hostname.append(host) #hostname or the socket
    return


def remove_connection_list(r_address):
    """removes connection from all list.
    loops through the known connections and if it matches removes it"""
    for i, item in enumerate(connectionaddress):# loops through connectionaddress list
        if item == r_address: # if item matches the r_address fed in
            del connectiondetails[i] # removes the index number of conneciton details
            del connectionaddress[i] # removes the index number of conneciton address
            del hostname[i] # removes the index number of the hostname
    return
         

def send_data(conn, data):
    """function that sends data across a socket,
    socket connection and data gets fed in, the length of data is then calculated.
    the socket sends a header file packed with struct that sends the total length and chunk size
    The data is sent in chunks of 4096 until the last chunk where only the required amount is sent.
    the function allows for raw bytes and strings to be sent for multiple data types to be sent.
    """
    total_length = len(data) # calculates the total length
    chunk_size = 4096 # sets the chunk size
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
    return received_data


def send_data_loadingbar(conn, data):
    """function that sends data across a socket,
    socket connection and data gets fed in, the length of data is then calculated.
    the socket sends a header file packed with struct that sends the total length and chunk size
    The data is sent in chunks of 4096 until the last chunk where only the required amount is sent.
    the function allows for raw bytes and strings to be sent for multiple data types to be sent.
    This function has a built in loading bar to track how much of teh data has been sent.
    """
    total_length = len(data) # calcuates the length of the data
    chunk_size = 4096 # sets the chunk size
    conn.sendall(struct.pack('!II', total_length, chunk_size)) # sends a header of the total_length and chunksize
    for i in tqdm(range(0, total_length, chunk_size), desc ="DataSent", colour="#39ff14"): # sets the length of the loadingbar based on total_length
        end_index = i + chunk_size if i + chunk_size < total_length else total_length # calcualtes how much data to be sent
        chunk = data[i:end_index] # sets the amount of data to be sent
        try:
            conn.sendall(chunk.encode()) #tries to send the data encoded
        except AttributeError:
            conn.sendall(chunk) #if the data cant be encoded sent it as it is. 
    return


