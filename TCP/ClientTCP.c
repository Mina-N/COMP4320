/*
** ClientTCP.c -- a stream socket client
*/

#include "TCP.h"

/* Get sockaddr, IPv4 or IPv6: */


void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Display the contents of the buffer */
void displayBuffer(char *Buffer, int length)
{
	int currentByte, column;
	currentByte = 0;
	printf("\n>>>>>>>>>>>> Content in hexadecimal <<<<<<<<<<<\n");
	while (currentByte < length)
	{
		printf("%3d: ", currentByte);
		column =0;
		while ((currentByte < length) && (column < 10))
		{
			printf("%2x ", Buffer[currentByte]);
			column++;
			currentByte++;
		}
		printf("\n");
	}
		printf("\n\n");
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes, random_num = 1;
	struct addrinfo hints, *servinfo, *p;
	int rv, send_message;
	char s[INET6_ADDRSTRLEN];
	int opCode, op1, op2;
	struct message_request req;
	char buf[MAXDATASIZE];
	struct timeval st, et;
	char *portNumber;
	char *message_buf;

	if (argc != 3) { //error entering in command line prompt: client servername
	    fprintf(stderr,"usage: client serverName portNumber\n");
	    exit(1);
	}

	portNumber = argv[2];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; //set address family to AF_UNSPEC
	hints.ai_socktype = SOCK_STREAM; //set socket type to SOCKET_STREAM, which provides reliable, two-way, connected-based byte stream

	if ((rv = getaddrinfo(argv[1], portNumber, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	/* Loop through all the results and connect to the first we can */
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure

	printf ("To send a message, press any number other than 2. To exit, press 2. ");
	scanf("%d", &send_message);

	while (send_message != 2)
	{

		/*START USER INPUT AND MESSAGE CONSTRUCTION*/
		printf ("Please enter the opcode: ");
	  scanf ("%d", &opCode);
		req.op_code = opCode;
		printf ("Please enter operand 1: ");
	  scanf ("%d", &op1);
		req.op_1 = htons(op1);
		req.op_2 = 0; //TODO: Is this ok?
		req.num_operands = 1;

		if (opCode != 6)
		{
			printf ("Please enter operand 2: ");
		  scanf ("%d", &op2);
			req.op_2 = htons(op2);
			req.num_operands = 2;
		}

		/* Finish forming the message, or the req struct */
		req.total_message_length = REQUEST_BYTES;
		req.request_id = random_num;
		random_num++;
		message_buf = (char *)&req;
		displayBuffer(message_buf, 8);

		/*END USER INPUT AND MESSAGE CONSTRUCTION*/


		//TODO: Check message size
		/*Send message to the server */
		if (send(sockfd, &req, sizeof(req), 0) < 0)
		{
		    perror("send");
		    exit(1);
		}
		gettimeofday(&st,NULL);
		printf("client: Message sent ");

		/* Listen for response from server */
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("recv");
		    exit(1);
		}
		struct message_response resp;
		memcpy(&resp, buf, sizeof(struct message_response));
		displayBuffer(buf, 7);
		gettimeofday(&et,NULL);
		int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
		/* Print out the request ID, the response, and the round trip time */
		printf("Request ID received: %d\n", resp.request_id);
		printf("Response received: %d\n", ntohl(resp.result));
		printf("Round trip time in microseconds: %d\n", elapsed);
		/* Prompt the user for a new request */
		printf ("To send another message, press any number other than 2. To exit, press 2. ");
		scanf("%d", &send_message);
	}

	close(sockfd);
	return 0;
}
