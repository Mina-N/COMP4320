import socket
import struct
import sys
import select
import threading
from binary_addition import *


def hex_to_ip_decimal(hex_data):
  ipaddr = "%i.%i.%i.%i" % (int(hex_data[6:8],16),int(hex_data[4:6],16),int(hex_data[2:4],16),int(hex_data[0:2],16))
  return str(ipaddr)

def read_udp(s, slaveRID, forwarding_addr):
    #ready = select.select([s], [], [], 15)
    #if ready[0]:
    while(1):
        udp_message, addr = s.recvfrom(65535)
        message_unpacked = struct.unpack('<BLBBB64sB', udp_message)
        #message_unpacked = struct.unpack('<b', udp_message)
        gid_sendingSlave = message_unpacked[0]
        magicNumberMaster = message_unpacked[1]
        ttl = message_unpacked[2]
        rid_dest = message_unpacked[3]
        rid_source = message_unpacked[4]
        message = message_unpacked[5]
        checksum = message_unpacked[6]

        #Calculate checksum, and ensure that message was not corrupted.
        list_of_bytes = []
        list_of_bytes.append(truncateBitString(bin(gid_sendingSlave), 1))

        #Split magic number into four bytes and add to list.
        magicNumberMasterBinary = truncateBitString(bin(magicNumberMaster), 4)
        list_of_bytes.append(magicNumberMasterBinary[0:8])
        list_of_bytes.append(magicNumberMasterBinary[8:16])
        list_of_bytes.append(magicNumberMasterBinary[16:24])
        list_of_bytes.append(magicNumberMasterBinary[24:32])

        list_of_bytes.append(truncateBitString(bin(ttl), 1))
        list_of_bytes.append(truncateBitString(bin(rid_dest), 1))
        list_of_bytes.append(truncateBitString(bin(rid_source), 1))

        #Split message into bytes and add to list.
        numBytesMessage = len(message.encode('utf-8'))
        messageBinary = ''.join(format(ord(x), 'b') for x in message)
        len_diff = (64 * 8) - len(messageBinary)
        messageBinary += "0"*len_diff

        i = 0
        while i < (64 * 8):
            message_splice = messageBinary[i:i + 8]
            list_of_bytes.append(message_splice)
            i += 8

        calculatedChecksum = calculateChecksumWithoutInverting(list_of_bytes[0], list_of_bytes[1])
        for i in range(2, len(list_of_bytes)):
            calculatedChecksum = calculateChecksumWithoutInverting(calculatedChecksum, list_of_bytes[i])
        newChecksum = bit_not(int(calculatedChecksum,2))
        finalChecksum = truncateBitString(newChecksum, 1)

        #If message was corrupted, print error message.
        if (finalChecksum != truncateBitString(bin(checksum), 1)):
            print("Message was corrupted. Message will be dropped.\n")
        else:
            #Is message for me? Print it out.
            if (rid_dest == slaveRID):
                print("The payload is: ")
                print(message)
            #Is message not for me? Forward it if ttl is greater than 1.
            else:
                if (ttl > 1):
                    #Decrement TTL
                    ttl -= 1
                    #Calculate new checksum
                    list_of_bytes[5] = truncateBitString(bin(ttl), 1)
                    calculatedChecksum = calculateChecksumWithoutInverting(list_of_bytes[0], list_of_bytes[1])
                    for i in range(2, len(list_of_bytes)):
                        calculatedChecksum = calculateChecksumWithoutInverting(calculatedChecksum, list_of_bytes[i])
                    newChecksum = bit_not(int(calculatedChecksum,2))
                    finalChecksum = truncateBitString(newChecksum, 1)
                    #Create a new udp_message.
                    udp_message = struct.pack('>BLBBB64sB', gid_sendingSlave, magicNumberMaster, ttl, rid_dest, rid_source, message, int(finalChecksum, 2))
                    s.sendto(udp_message, forwarding_addr)


def prompt_message(gid_slave, magicNumber, slaveRID, sock_udp, forwarding_addr):

    message = raw_input("Please enter a message, or enter exit if no message should be sent.")
    if (message != "exit"):
        while (len(message) > 64 * 8):
            message = input("Please enter a shorter message.\n")
        ringIDDest = input("Please enter a ring ID.\n")
        ttl = 255
        list_of_bytes = []

        list_of_bytes.append(truncateBitString(bin(gid_slave), 1))
        #Split magic number into four bytes and add to list.
        magicNumberMasterBinary = truncateBitString(bin(magicNumber), 4)
        list_of_bytes.append(magicNumberMasterBinary[0:8])
        list_of_bytes.append(magicNumberMasterBinary[8:16])
        list_of_bytes.append(magicNumberMasterBinary[16:24])
        list_of_bytes.append(magicNumberMasterBinary[24:32])
        list_of_bytes.append(truncateBitString(bin(ttl), 1))
        list_of_bytes.append(truncateBitString(bin(ringIDDest), 1))
        list_of_bytes.append(truncateBitString(bin(slaveRID), 1))

        #print('gid_slave: ' + str(truncateBitString(bin(gid_slave), 1)) + "\n")
        #print('magic number1: ' + str(magicNumberMasterBinary[0:8]) + "\n")
        #print('magic number2: ' + str(magicNumberMasterBinary[8:16]) + "\n")
        #print('magic number3: ' + str(magicNumberMasterBinary[16:24]) + "\n")
        #print('magic number4: ' + str(magicNumberMasterBinary[24:32]) + "\n")
        #print('ttl: ' + str(truncateBitString(bin(ttl), 1)) + "\n")
        #print('ringIDDest: ' + str(truncateBitString(bin(ringIDDest), 1)) + "\n")
        #print('slaveRID: ' + str(truncateBitString(bin(slaveRID), 1)) + "\n")

        #Split message into bytes and add to list.
        numBytesMessage = len(message.encode('utf-8'))
        messageBinary = ''.join(format(ord(x), 'b') for x in message)
        len_diff = (64 * 8) - len(messageBinary)
        messageBinary += "0"*len_diff
        #print('messageBinary: ' + str(messageBinary) + '\n')

        message_counter = 1
        i = 0
        while i < (64 * 8):
            message_splice = messageBinary[i:i + 8]
            list_of_bytes.append(message_splice)
            #print('message ' + str(message_counter) + ': ' + str(message_splice) + "\n")
            i += 8
            message_counter += 1

        calculatedChecksum = calculateChecksumWithoutInverting(list_of_bytes[0], list_of_bytes[1])
        for i in range(2, len(list_of_bytes)):
            calculatedChecksum = calculateChecksumWithoutInverting(calculatedChecksum, list_of_bytes[i])
        newChecksum = bit_not(int(calculatedChecksum,2))
        finalChecksum = truncateBitString(newChecksum, 1)
        #print('Final Checksum: ' + str(finalChecksum) + "\n")
        #print(str(gid_slave) + "\n")
        #print(str(magicNumber) + "\n")
        #print(str(ttl) + "\n")
        #print(str(ringIDDest) + "\n")
        #print(str(slaveRID) + "\n")
        #print(message + "\n")
        #print(str(int(finalChecksum, 2)) + "\n")

        message = struct.pack('>BLBBB64sB', gid_slave, magicNumber, ttl, ringIDDest, slaveRID, message, int(finalChecksum, 2))
        numBytes = sock_udp.sendto(message, forwarding_addr)
        #print(str("Bytes Sent: " + str(numBytes) + "\n"))


addAnotherSlave = 1
magicNumber = 0x4A6F7921
if (len(sys.argv) != 3):
    print("usage: slave masterhostname portnumber")
    sys.exit(1)

master_hostname = sys.argv[1]
listening_port = int(sys.argv[2])

server_address = (master_hostname, listening_port)

sock_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_tcp.connect(server_address)
# prompt for GID
gid_slave = input("Please enter GID: ")
message_repack = struct.pack('>bl', gid_slave, magicNumber) #gid is one byte, magicNumber is four bytes
print >>sys.stderr, 'sending slave message to master'
sock_tcp.send(message_repack)
# Look for the response
longest_message_size = 100
print("Waiting for server (master) message.\n")
message = sock_tcp.recv(longest_message_size)
if not message:
    print("No message received. Closing socket.\n")
    sock_tcp.close()
message_unpacked = struct.unpack('<blbl', message)
gid_master = message_unpacked[0]
magicNumberMaster = hex((message_unpacked[1]))
slaveRID = message_unpacked[2]
nextSlaveIP = hex(message_unpacked[3])
nextSlaveIP = nextSlaveIP[2:]
print("GID of master: " + str(gid_master))
print("Magic Number: " + str(magicNumberMaster))
print("My RID: " + str(slaveRID))
dottedDecimal = hex_to_ip_decimal(nextSlaveIP)
print("IP Address of Next Slave: " + dottedDecimal)

#Calculate forwarding port
forwarding_port = 10010 + (gid_master * 5) + (slaveRID - 1)
#Calculate listening port
listening_port = 10010 + (gid_master * 5) + slaveRID
#create a sending and forwarding datagram service
sock_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
listening_addr = ('', listening_port)
sock_udp.bind(listening_addr)
forwarding_addr = (dottedDecimal, forwarding_port)

try:
    threading.Thread(target=read_udp, args=(sock_udp,slaveRID,forwarding_addr)).start()
    while(1):
        prompt_message(gid_slave, magicNumber, slaveRID, sock_udp, forwarding_addr)
        # starting thread 1
        #t1.start()
        # starting thread 2
        #t2.start()
        # wait until thread 1 is completely executed
        #t1.join()
        # wait until thread 2 is completely executed
        #t2.join()
        #ready_read, ready_write, exceptional = select.select([sock_udp1, sock_udp2], [], [], None)

        #for s in ready_read:
        #    if s == sock_udp1:
        #        read_udp(s, sock_udp2)
        #    else:

finally:
    print >>sys.stderr, 'closing socket'
    sock_tcp.close()
