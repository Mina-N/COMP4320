import socket
import struct
import sys

magicNumber = 0x4A6F7921
addAnotherSlave = 1

if (len(sys.argv) != 3):
    print("usage: slave masterhostname portnumber")
    sys.exit(1)

master_hostname = sys.argv[1]
listening_port = int(sys.argv[2])

server_address = (master_hostname, listening_port)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(server_address)

try:
    while(addAnotherSlave == 1):
        # prompt for GID
        gid_slave = input("Please enter GID: ")
        message_repack = struct.pack('>bl', gid_slave, magicNumber) #gid is one byte, magicNumber is four bytes
        print >>sys.stderr, 'sending slave message to master'
        sock.send(message_repack)

        # Look for the response
        longest_message_size = 100 #TODO: CHANGE
        print("Waiting for server (master) message.\n")
        message = sock.recv(longest_message_size)
        if not message:
            print("No message received. Closing socket.\n")
            sock.close()
        message_unpacked = struct.unpack('>blbl', message)
        print(message_unpacked)
        gid_master = message[0]
        magicNumberMaster = message[1]
        slaveRID = message[2]
        nextSlaveIP = message[3]
        print("GID of master: " + str(gid_master) + "\n")
        print("My RID: " + str(slaveRID) + "\n")
        print("IP Address of Next Slave: " + str(nextSlaveIP) + "\n")
        addAnotherSlave = input("Press 1 to add another slave, or 0 to exit. ")

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
