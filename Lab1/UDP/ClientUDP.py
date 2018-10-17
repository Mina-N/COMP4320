#
# talker.c -- a datagram "client" demo
# 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "10022"	// the port users will be connecting to
SERVERPORT = "10022"

def main():

	# reconstruct the main method
	# check for three inputs

	operand1 = int(raw_input("Enter first operand: "))
	op_code = int(raw_input("Enter op code: "))
	operand2 = int(raw_input("Enter second operand: "))

	





# int main(int argc, char *argv[]) //command line prompt: client servername port#
# {
# 	int sockfd;
# 	struct addrinfo hints, *servinfo, *p;
# 	int rv;
# 	int numbytes;

# 	if (argc != 3) { // if the user doesn't enter in the correct command prompt
# 		fprintf(stderr,"usage: talker hostname message\n"); //print error
# 		exit(1);
# 	}

# 	memset(&hints, 0, sizeof hints); //create memory the size of hints struct
# 	hints.ai_family = AF_UNSPEC; //assign the address family of hints struct to AF_UNSPEC
# 	hints.ai_socktype = SOCK_DGRAM; //assign the socket type of hints struct to SOCK_DGRAM, which supports connectionless datagrams

# 	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) { //argv[1] is servername
# 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); //if this does not return 0, error getting server data
# 		return 1;
# 	}

# 	// loop through all the results and make a socket
# 	for(p = servinfo; p != NULL; p = p->ai_next) {
# 		if ((sockfd = socket(p->ai_family, p->ai_socktype,
# 				p->ai_protocol)) == -1) {
# 			perror("talker: socket");
# 			continue;
# 		}

# 		break;
# 	}

# 	if (p == NULL) { //client failed to create socket
# 		fprintf(stderr, "talker: failed to create socket\n");
# 		return 2;
# 	}

# 	if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0, //client failed to send message
# 			 p->ai_addr, p->ai_addrlen)) == -1) {
# 		perror("talker: sendto");
# 		exit(1);
# 	}

# 	freeaddrinfo(servinfo);

# 	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]); //client successfully sent message
# 	close(sockfd);

# 	return 0;
# }
