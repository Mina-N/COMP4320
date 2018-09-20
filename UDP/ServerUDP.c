/*
** listener.c -- a datagram sockets "server" demo
*/

// header file for ServerUDP.c
#include "ServerUDP.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	struct message_receive buf;
	char* buffer;
	char request_ID = 0;
	char error_code = 0;
	struct message_send *message;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char opCode;
	short op1;
	short op2;
	float result;
	enum opCodes operations;
	char* portNumber;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM; // datagram for UDP
	hints.ai_flags = AI_PASSIVE; // use my IP

	if(argc != 2) {
		fprintf(stderr, "usage: server portnumber");
		exit(1);
	}

	portNumber = argv[1];

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;

	while(1) {

		if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			// close(sockfd);
			exit(1);
		}

		printf("Something was found");

		printf("listener: packet is %d bytes long\n", numbytes);
		buffer[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buffer);

		opCode = buf.op_code;
		op1 = ntohs(buf.op_1);
		op2 = ntohs(buf.op_2);

		if(opCode == 6) {
			op2 = 0;
		}

		// check errors
		error_code = checkErrors(buf, opCode);
		
		if(error_code == 0) {
			result = getResult(op1, opCode, op2);
		}
		else {
			result = 0;
		}

		message->total_message_length = RESPONSE_BYTES;
		message->result = htonl(result);
		message->request_ID = buf.request_id;
		message->error_code = error_code;

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));
			
		printf("listener: packet is %d bytes long\n", numbytes);
		char packet[REQUEST_BYTES];
		packet[0] = buf.total_message_length;
		packet[1] = buf.request_id;
		packet[2] = buf.op_code;
		packet[3] = buf.num_operands;
		packet[4] = ntohs(buf.op_1);
		packet[5] = ntohs(buf.op_2);
		packet[6] = '\0';
		printf("listener: packet contains \"%s\"\n", packet);

		if(sendto(sockfd, message, sizeof(message), error_code, 
			(const struct sockaddr *) &their_addr, addr_len) == -1) {
			perror("sendto");
			close(sockfd);
			exit(1);
		};

		printf("Message sent to client");
	}

	close(sockfd);
	return 0;
}


float getResult(short op1, char opCode, short op2) {
	float result = 0;
	switch(opCode) {
		case addOp:
			result = op1 + op2;
			break;
		case subOp:	
			result = op1 - op2;
			break;
		case orOp:
			result = op1 | op2;
			break;
		case andOp:
			result = op1 & op2;
			break;
		case shRightOp:
			result = op1 >> op2;
			break;
		case shLeftOp:
			result = op1 << op2;
			break;
		case notOp:
			result = ~op1;
			break;
	}

	return result;
}

char checkErrors(struct message_receive buf, char opCode) {
	// check op code
	int isError = 0;
	if(opCode < 0 || opCode > 6) {
		isError = 1;
	}

	if(sizeof(buf) != buf.total_message_length){
		isError = 1;
	}

	return isError == 1 ? 127 : 0;

}

