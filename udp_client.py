#!/usr/bin/env python
import socket
import sys
import struct
import random
import time
from struct import *

port = 10022
max_data_size = 100
request_id = 1

# check to see if you have valid command line inputs
if (len(sys.argv) != 3):
     sys.stderr.write("usage: talker hostname message\n")
     exit(1)

#set the values of the two argument into variables
serverName = sys.argv[1]
portNumber = int(sys.argv[2])

#Ask user to send a message or exit
send_message = int(input("To send a message, press 1. To exit, press 2."))
print (type(send_message))
while(send_message != 2):

    # Create a datagram socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    except socket.error:
        sys.stderr.write('talker: Failed to create socket')
        sys.exit()


    total_message_length = 8
    op_code = 8
    num_operands = 8
    op_1 = 16
    op_2 = 16

    opCode = int(input("Please enter an Opcode between 0 and 6 to start: "))
    #while (opCode < 0 or opCode > 6):
        #opCode = eval(input("Invalid Entry: Please enter an Opcode between 0 and 6: "))

    op1 = int(input("Please enter operand 1: "))

    #total_message_length = 8

    if (opCode == 6):
        num_operands = 1
        op2 = 0

    else:
        num_operands = 2
        op2 = int(input("Please enter operand 2: "))

    #op1 = eval(input("Please enter operand 1: "))

    op_code = opCode

    op_1 = op1

    op_2 = op2

    #Pack the message into a Struct in Big Endian
    message = pack('!bbbbhh', total_message_length, request_id , op_code, num_operands, op_1, op_2)


    #Send a message and measure time it takes to send a message
    send_start = (time.time() * 1000000000)
    send = s.sendto(message, (serverName, portNumber))
    send_time = ((time.time() * 1000000000) - send_start)
    print ("Time to send in nano seconds: {}".format(send_time))

    #Check to see if you can send
    if(send <= 0):
        sys.stderr.write("Send\n")
        exit(1)

    print ("Connecting to Server: {}".format(serverName))
    #text_string = message.decode('utf-8')

    print ("Sending Message... ")

    #Make sure valid msg, recv the message and measure time it takes to recv a message
    recv_start = (time.time() * 1000000000)
    try:
        numBytes = s.recv(max_data_size)
    except socket.error:
        sys.stderr.write("Error: Received\n")
        sys.exit()

    recv_time = ((time.time() * 1000000000) - recv_start)
    rount_trip_time = recv_time - send_time
    print ("Time to recv in nano seconds: {}".format(recv_time))

    msg = unpack('!bbbl', numBytes)
    print ("Message being received is: ")
    print (msg)

    round_trip_time = send_time - recv_time
    print ("Total Round Trip Time in nano seconds is: {}".format(abs(round_trip_time)))

    #not sure about this
    if (numBytes == ''):
        sys.stderr.write("nothing received\n")

    request_id += 1

s.close()
