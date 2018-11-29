
import socket 
import sys
from struct import *

port = 10022
max_data_size = 65
request_id = 1

# check to see if you have valid command line inputs
if (len(sys.argv) != 3):
     sys.stderr.write("usage: talker hostname message\n")
     exit(1)

#set the values of the two argument into variables
serverName = sys.argv[1]
portNumber = int(sys.argv[2])

# Create a datagram socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except socket.error:
    sys.stderr.write('talker: Failed to create socket')
    sys.exit()

rid = 10
message = "How bout naaaoooo"

message = pack('<bs', rid, message)
send = s.sendto(message, (serverName, portNumber))

#Check to see if you can send
if(send <= 0):
    sys.stderr.write("Send\n")
    exit(1)

print ("Connecting to Server: {}".format(serverName))
#text_string = message.decode('utf-8')

print ("Sending Message... ")

try:
    numBytes = s.recv(max_data_size)
except socket.error:
    sys.stderr.write("Error: Received\n")
    sys.exit()

print(numBytes)
msg = unpack('>bi', 5)

print("Ring ID: {}".format(msg[1]))
print("Message: {}".format(msg[2]))
