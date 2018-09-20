/*
** listener.c -- a datagram sockets "server" demo
*/

// header file for ServerUDP.c
#include "ServerUDP.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	struct message_receive buf;
	char buffer[MAXDATASIZE];
	char request_ID = 0;
	char error_code = 0;
	struct message_send message;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char opCode;
	short op1;
	short op2;
	int32_t result;
	enum opCodes operations;
	char *portNumber;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	// set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM; // datagram for UDP
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if (argc != 2)
	{
		fprintf(stderr, "usage: server portnumber");
		exit(1);
	}

	portNumber = argv[1];

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	while (1)
	{

		addr_len = sizeof their_addr;

		if ((numbytes = recvfrom(sockfd, buffer, 9, 0,
								 (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			// close(sockfd);
			exit(1);
		}

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));

		printf("listener: packet is %d bytes long\n", numbytes);
		buffer[numbytes] = '\0';

		printf("listener: packet contains:\n");
		for(int i = 0; i < 8; i++) {
			printf("%x ", buffer[i]);
		}

		printf("\n");

		// set message to message_receive struct
		buf.total_message_length = buffer[0];
		buf.request_id = buffer[1];
		buf.op_code = buffer[2];
		buf.num_operands = buffer[3];
		buf.op_1 = htons((buffer[5] << 8) | buffer[4]);
		buf.op_2 = htons((buffer[7] << 8) | buffer[6]);

		if(buf.op_code == 6) {
			buf.op_2 = 0;
		}

		// // check errors
		error_code = checkErrors(buf, buf.op_code);

		if(error_code == 0) {
			result = getResult(buf.op_1, buf.op_code, buf.op_2);
		}
		else {
			result = 0;
		}

		message.total_message_length = RESPONSE_BYTES;
		message.result = htonl(result);
		message.request_ID = buf.request_id;
		message.error_code = error_code;
		
		if (sendto(sockfd, (char*)&message, sizeof(message), error_code,
				   (const struct sockaddr *)&their_addr, addr_len) == -1)
		{
			perror("sendto");
			exit(1);
		};
	}

	close(sockfd);
	return 0;
}

int32_t getResult(short op1, char opCode, short op2)
{
	int32_t result = 0;
	switch (opCode)
	{
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

char checkErrors(struct message_receive buf, char opCode)
{
	// check op code
	int isError = 0;
	int message_length = opCode == 6 ? 6 : 8;
	if (opCode < 0 || opCode > 6)
	{
		isError = 1;
	}

	if (message_length != buf.total_message_length)
	{
		isError = 1;
	}

	if(isError) {
		printf("An error has occurred!");
	}

	return isError == 1 ? 127 : 0;
}
