/*
** listener.c -- a datagram sockets "server" demo
*/

// header file for ServerUDP.c
#include <ServerUDP.h>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	struct message_receive *buf;
	int request_ID = 0;
	int error_code = 0;
	struct message_send *message;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int opCode;
	int op1;
	int op2;
	int result;
	enum opCodes operations;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM; // datagram for UDP
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
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

		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		opCode = buf->op_code;
		op1 = buf->op_1;

		// check errors
		checkErrors(buf, &op2, &error_code);
		
		
		if(error_code == 0) {
			getResult(op1, opCode, op2, &result);
		}
		else {
			result = 0;
		}

		message->total_message_length = 7;
		message->result = result;
		message->request_ID = ++request_ID;
		message->error_code = error_code;

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));
			
		printf("listener: packet is %d bytes long\n", numbytes);
		printf("listener: packet contains \"%s\"\n", buf);

		if(sendto(sockfd, message, sizeof message, error_code, 
			(const struct sockaddr *) &their_addr, addr_len) == -1) {
			perror("sendto");
			exit(1);
		};

		printf("Message sent to client");
	}

	close(sockfd);
	
	return 0;
}


void getResult(int op1, int opCode, int op2, int result) {
	switch(opCode) {
		case addOp:
			add(op1, op2, &result);
			break;
		case subOp:
			subtract(op1, op2, &result);
			break;
		case orOp:
			OR(op1, op2, &result);
			break;
		case andOp:
			AND(op1, op2, &result);
			break;
		case shRightOp:
			rShift(op1, op2, &result);
			break;
		case shLeftOp:
			lShift(op1, op2, &result);
			break;
		case notOp:
			NOT(op1, &result);
			break;
	}
}

void checkErrors(struct message_receive *buf, int op2, int error_code) {
	// check op code
	if(buf->op_code == 6) {
		op2 = 0;
	}
	else if(buf->op_code > 0 || buf->op_code < 6) {
		op2 = buf->op_2;
	}
	else {
		error_code = 127;
	}

	if(sizeof buf != buf->total_message_length){
		error_code = 127;
	}
}

void add(int op1, int op2, int result) {
	result = op1 + op2;
}

void subtract(int op1, int op2, int result) {
	result = op1 - op2;
}

void OR(int op1, int op2, int result) {
	result = op1 | op2;
}

void AND(int op1, int op2, int result) {
	result = op1 & op2;
}

void rShift(int op1, int op2, int result) {
	result = op1 >> op2;
}

void lShift(int op1, int op2, int result) {
	result = op1 << op2;
}

void NOT(int op, int result) {
	result = ~op;
}
