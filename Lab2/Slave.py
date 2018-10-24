import socket
import struct
import sys


def hex_to_ip_decimal(hex_data):
  ipaddr = "%i.%i.%i.%i" % (int(hex_data[6:8],16),int(hex_data[4:6],16),int(hex_data[2:4],16),int(hex_data[0:2],16))
  return ipaddr

magicNumber = 0x4A6F7921
addAnotherSlave = 1

if (len(sys.argv) != 3):
    print("usage: slave masterhostname portnumber")
    sys.exit(1)

master_hostname = sys.argv[1]
listening_port = int(sys.argv[2])

server_address = (master_hostname, listening_port)



try:
    while(addAnotherSlave == 1):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(server_address)
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

        message_unpacked = struct.unpack('<blbl', message)
        gid_master = message_unpacked[0]
        magicNumberMaster = hex((message_unpacked[1]))
        slaveRID = message_unpacked[2]
        nextSlaveIP = hex(message_unpacked[3])
	nextSlaveIP = nextSlaveIP[2:]
        print("GID of master: " + str(gid_master) + "\n")
        print("Magic Number: " + str(magicNumberMaster) + "\n")
        print("My RID: " + str(slaveRID) + "\n")
        dottedDecimal = hex_to_ip_decimal(nextSlaveIP)
        print("IP Address of Next Slave: " + str(dottedDecimal) + "\n")
        addAnotherSlave = input("Press 1 to add another slave, or 0 to exit. ")

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
