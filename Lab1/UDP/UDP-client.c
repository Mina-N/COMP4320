/*
** talker.c -- a datagram "client" demo
*/

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

struct message_request
{
  uint8_t total_message_length;
  uint8_t request_id;
  uint8_t op_code;
  uint8_t num_operands;
  short op_1;
  short op_2;
} __attribute__((__packed__));

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	int rv;
	int numbytes;
	struct message_request req;
	char buffer[8];

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	req.total_message_length = 8;
	req.op_code = 0;
	req.num_operands = 2;
	req.request_id = 0;
	req.op_1 = htons(120);
	req.op_2 = htons(60);

	short op1 = htons(240);
	short op2 = htons(160);



	buffer[0] = 8;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 2;
	buffer[4] = op1 & 0xFF;
	buffer[5] = op1 >> 8;
	buffer[6] = op2 & 0xFF;
	buffer[7] = op2 >> 8;


	if ((numbytes = sendto(sockfd, buffer, sizeof(buffer), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);
	char message[100];
	socklen_t addr_len = sizeof their_addr;
	recvfrom(sockfd, message, 100, 0, (struct sockaddr *)&their_addr, &addr_len);

	printf("New message: \"%s\"\n", message);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);

	return 0;
}
