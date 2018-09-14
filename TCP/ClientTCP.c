/*
** ClientTCP.c -- a stream socket client
*/

#include "ClientTCP.h"

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
		printf("%3d: ", currentByte);column =0;
		while ((currentByte < length) && (column < 10))
		{
			printf("%2x ",Buffer[currentByte]);
			column++;
			currentByte++;
		}
		printf("\n");}printf("\n\n");
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	char message[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	int opCode;
	int op1;
	int op2;
	int send_message;

	if (argc != 3) { //error entering in command line prompt: client servername
	    fprintf(stderr,"usage: ./ClientTCP client hostname\n");
	    exit(1);
	}

	srand(time(NULL));
	int random_num = rand();

	printf ("To send a message, press 1. To exit, press 2.");
	scanf("%d", &send_message);

	while (send_message != 2)
	{
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC; //set address family to AF_UNSPEC
		hints.ai_socktype = SOCK_STREAM; //set socket type to SOCKET_STREAM, which provides reliable, two-way, connected-based byte stream

		if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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

		/* Prompt user for input */
		printf ("Please enter the opcode: ");

	  scanf ("%d", &opCode);

		while (opCode < 0 || opCode > 6 )
		{
		   printf ("Please enter an opcode between 1 and 6: ");
	 	   scanf("%d", &opCode);
		}

		printf ("Please enter operand 1: ");
	  	scanf ("%d", &op1);
		printf ("Please enter operand 2, or enter -1 to indicate that there is no operand 2: ");
	  	scanf ("%d", &op2);

		/* Form the message */
		struct message_request req;
		if (op2 == -1)
		{
			req.total_message_length = 6;
			req.num_operands = 1;
		}
		else
		{
			req.total_message_length = 8;
			req.num_operands = 2;
		}

		message[0] = req.total_message_length;
	  req.request_id = random_num;
		random_num++;
		message[1] = req.request_id;
		req.op_code = opCode;
		message[2] = req.op_code;
		message[3] = req.num_operands;
		req.op_1 = op1;
		req.op_2 = op2;
		message[4] = req.op_1;
		if (op2 != -1)
		{
			message[5] = req.op_2;
			message[6] = '\0';
			char *message_ptr = message;
			displayBuffer(message_ptr, 6);
		}
		else
		{
			message[5] = '\0';
			char *message_ptr = message;
			displayBuffer(message_ptr, 5);
		}

		const struct message_request *req_ptr = &req;

		/*Send message to the server */
		if (send(sockfd, req_ptr, req.total_message_length, 0) < 0)
		{
		    perror("send");
		    exit(1);
		}

		printf("client: Message sent ");

		/* Listen for response from server */
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("recv");
		    exit(1);
		}

		buf[numbytes] = '\0';
		printf("client: Message received '%s'\n",buf);

		/* Print out the request ID, the response, and the round trip time */ //TODO

		/* Prompt the user for a new request */
		printf ("To send another message, press 1. To exit, press 2.");
		scanf("%d", &send_message);
	}

	close(sockfd);
	return 0;
}
