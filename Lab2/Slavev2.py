import socket
import struct
import sys
from binary_addition import *


def hex_to_ip_decimal(hex_data):
  ipaddr = "%i.%i.%i.%i" % (int(hex_data[6:8],16),int(hex_data[4:6],16),int(hex_data[2:4],16),int(hex_data[0:2],16))
  return ipaddr

def read_udp(s, s2):
    udp_message = s.recvfrom(100)
    message_unpacked = struct.unpack('<blbbbsb', udp_message)
    gid_sendingSlave = message_unpacked[0]
    magicNumberMaster = message_unpacked[1]
    ttl = message_unpacked[2]
    rid_dest = message_unpacked[3]
    rid_source = message_unpacked[4]
    message = message_unpacked[5]
    checksum = message_unpacked[6]

    #Calculate checksum, and ensure that message was not corrupted.
    list_of_bytes = []
    list_of_bytes.append(truncateBitString(bin(gid_sendingSlave)))

    #TODO: SPLIT MAGIC NUMBER INTO BYTES AND ADD TO LIST_OF_BYTES

    list_of_bytes.append(truncateBitString(bin(ttl)))
    list_of_bytes.append(truncateBitString(bin(rid_dest)))
    list_of_bytes.append(truncateBitString(bin(rid_source)))

    #TODO: SPLIT MESSAGE INTO BYTES AND ADD TO LIST_OF_BYTES

    list_of_bytes.append(truncateBitString(bin(checksum)))

    calculatedChecksum = calculateChecksum(list_of_bytes[0], list_of_bytes[1])
    for i in range(2, len(list_of_bytes)):
        calculatedChecksum = calculateChecksum(calculatedChecksum, list_of_bytes[i]) #TODO: verify checksum is computed correctly

    #TODO: If message was corrupted, print error message


    #Is message for me? Print it out.
    if (rid_dest == slaveRID):
        print("The payload is:\n")
        print(message)

    #Is message not for me? Forward it.
    else:
        #Decrement TTL
        ttl -= 1
        #TODO: Calculate new checksum
        #TODO: create new udp_message
        udp_message = ''
        s2.sendto(udp_message, addr)


magicNumber = 0x4A6F7921
addAnotherSlave = 1

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
print("IP Address of Next Slave: " + str(dottedDecimal))

#Calculate forwarding port
forwarding_port = 10010 + (gid_master * 5) + (slaveRID - 1)
#Calculate listening port
listening_port = 10010 + (gid_master * 5) + slaveRID
#create a sending and forwarding datagram service
sock_udp1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_udp1.bind('',listening_port)
sock_udp2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_udp2.bind(str(dottedDecimal),forwarding_port)
addr = (str(dottedDecimal), forwarding_port)

try:
    while(1):
        ready_read, ready_write, exceptional = select.select([sock_udp1, sock_udp2], [], [], None)

        for s in ready_read:
            if s == sock_udp1:
                read_udp(s, sock_udp2)

            else:
                ringIDDest = input("Please enter a ring ID.\n")
                message = input("Please enter a message.\n")
                while (len(message.encode('utf-8')) > 64): #TODO: check this
                    message = input("Please enter a shorter message.\n")

                ttl = 255 #TODO: check this
                checksum = 0 #TODO: compute checksum
                message = struct.pack('>blbbbsb', gid_slave, magicNumber, ttl, ringIDDest, slaveRID, message, checksum)

                #send message using sock_udp2


finally:
    print >>sys.stderr, 'closing socket'
    sock_tcp.close()
