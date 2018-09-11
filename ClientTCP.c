/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "10022" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

struct message_request //TODO: move to a .h file
{
  unsigned int  total_message_length;
  unsigned int request_id;
  unsigned int op_code;
  unsigned int num_operands;
  signed float op_1; // subject to change
  signed float op_2; //subject to change
  
} __attribute__((__packed__));


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
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) { //error entering in command line prompt: client servername
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}
	
	//TODO: use this data to alter how the client socket connects to the server
	string serverName = argv[1];
	int portNumber = argv[2];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; //set address family to AF_UNSPEC
	hints.ai_socktype = SOCK_STREAM; //set socket type to SOCKET_STREAM, which provides reliable, two-way, connected-based byte stream

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
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
	
	//prompt user for input
	//TODO: error checking
	printf ("Please enter the opcode: ");
  	scanf ("%s", opCode);
	printf ("Please enter operand 1: ");
  	scanf ("%s", op1);
	printf ("Please enter operand 2, or enter a space for no operand 2: ");
  	scanf ("%s", op2);
	typedef struct message_request req; 
	if (strcmp(op2, " ") == 0)
	{
	    req.total_message_length = 6;
            //TODO: initialize request_id to random value and increment for each request
            req.request_id = 0;
	    req.op_code = opCode;
	    req.num_operands = 1;
	    req.op_1 = op1;
	    req.op_2 = op2;	
	}
	else 
	{
	    req.total_message_length = 8;
            //TODO: initialize request_id to random value and increment for each request
            req.request_id = 0;
	    req.op_code = opCode;
	    req.num_operands = 1;
	    req.op_1 = op1;
	    req.op_2 = op2;			
	}
  	
	
	
	//TODO: send request to server here
	
	
	//try to receive response from server
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}

