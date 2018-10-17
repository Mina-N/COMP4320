import socket
import struct
import sys

if (len(sys.argv) != 2):
    sys.exit(1)

listening_port = int(sys.argv[1])
listening_adapter = ''
listening_endpoint = (listening_adapter, listening_port)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(listening_endpoint)
print("Listening for client connection.\n")
sock.listen(1)
sock, sender = sock.accept() # Return the TCP connection
print("Accepted client connection.\n")

while(True):

    longest_message_size = 100
    print("Waiting for client message.\n")
    message = sock.recv(longest_message_size)
    if not message:
        print("No message received. Closing socket.")
        break
    print >>sys.stderr, 'Client message received:\n'
    message_unpacked = struct.unpack('>bbbbhh', message)
    print(message_unpacked)
    total_message_length = message_unpacked[0]
    request_id = message_unpacked[1]
    op_code = message_unpacked[2]
    num_operands = message_unpacked[3]
    op_1 = message_unpacked[4]
    op_2 = message_unpacked[5]
    error_code = 0
    result = 0
    response_total_message_length = 7

    if (len(message) != total_message_length):
        error_code = 127
    if (op_code == 0):
        result = op_1 + op_2
    elif (op_code == 1):
        result = op_1 - op_2
    elif (op_code == 2):
        result = op_1 | op_2
    elif (op_code == 3):
        result = op_1 & op_2
    elif (op_code == 4):
        result = op_1 >> op_2
    elif (op_code == 5):
        result = op_1 << op_2
    elif (op_code == 6):
        result = ~op_1
    else:
        error_code = 127

    message_repack = struct.pack('>bbbl', response_total_message_length, request_id, error_code, result)
    print 'Response to client request :', struct.unpack('>bbbl', message_repack)
    sock.send(message_repack)

sock.close()
