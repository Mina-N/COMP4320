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
send_message = input("To send a message, press 1. To exit, press 2.")
while(send_message != 2):
    #check if its an ipv4 or ipv6 Address
    ipType = 0
    try:
        socket.inet_aton(serverName)
        ipType = 4
    except socket.error:
        ipType = 6

    if ipType != 4:
        try:
            socket.inet_pton(socket.AF_INET6, serverName)
        except socket.error:
            ipType = 0

    if(ipType == 0):
        sys.stderr.write('Not a valid ip\n')
        exit(1)

    if(ipType == 4):
        ip = socket.inet_aton(serverName)
        family = socket.AF_INET
    else:
        ip = socket.inet_pton(socket.AF_INET6, serverName)
        family = socket.AF_INET6


    # Create a datagram socket
    try:
        s = socket.socket(family, socket.SOCK_DGRAM)
    except socket.error:
        sys.stderr.write('talker: Failed to create socket')
        sys.exit()


    total_message_length = 8
    op_code = 8
    num_operands = 8
    op_1 = 16
    op_2 = 16

    opCode = eval(input("Please enter an Opcode between 0 and 6: "))
    while (opCode < 0 or opCode > 6):
        opCode = eval(input("Invalid Entry: Please enter an Opcode between 0 and 6: "))

    op1 = eval(input("Please enter operand 1: "))

    if (opCode == 6):
        total_message_length = 6
        num_operands = 1
        op2 = 0

    else:
        total_message_length = 8
        num_operands = 2
        op2 = eval(input("Please enter operand 2: "))

    #op1 = eval(input("Please enter operand 1: "))

    op_code = opCode

    op_1 = op1

    op_2 = op2

    #Pack the message into a Struct in Big Endian
    message = pack('!bbbbhh', total_message_length, request_id , op_code, num_operands, op_1, op_2)

    #Send a message and measure time it takes to send a message
    send_start = time.time()
    send = s.sendto(message, (serverName, portNumber))
    send_time = time.time()- send_start
    print ("Time to send: {}".format(send_time))

    #Check to see if you can send
    if(send <= 0):
        sys.stderr.write("Send\n")
        exit(1)

    print("Connecting to Server: {}".format(serverName))
    print ("Message being sent is: {}".format(message))

    #Make sure valid msg, recv the message and measure time it takes to recv a message
    recv_start = time.time()
    try:
        numBytes = s.recv(max_data_size)
    except socket.error:
        sys.stderr.write("Error: Recieved\n")
        sys.exit()

    recv_time = time.time()- recv_start
    print ("Time to recv: {}".format(recv_time))

    print ("Message being received is: {}".format(numBytes))

    #not sure about this
    if (numBytes == ''):
        sys.stderr.write("recv\n")

    request_id += 1
    s.close()

exit(1)
