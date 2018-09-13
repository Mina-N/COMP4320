/*
** client.c -- a stream socket client demo
*/

// Header file for ClientTCP.c"
#include <ClientTCP.h>

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
	
	char* serverName = argv[1];
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
	

	u_int8_t opCode;
	int16_t op1;
	int16_t op2;
	//prompt user for input
	printf ("Please enter the opcode: ");
  	scanf ("%d", opCode);
	
	while (opCode < 0 || opCode > 6 )
	{
	   printf ("Please enter an opcode between 1 and 6: ");
 	   scanf("%d", opCode);	
	}
		
	printf ("Please enter operand 1: ");
  	scanf ("%d", op1);	
	printf ("Please enter operand 2, or enter -1 to indicate that there is no operand 2: ");
  	scanf ("%d", op2);
	
	if (op2 == -1)
	{
	    message_request req;
	    req.total_message_length = 6;
            req.request_id = rand(); ///initialize request_id to random value
	    req.op_code = opCode;
	    req.op_1 = op1;
	}
	else 
	{
	    message_request req;
	    req.total_message_length = 8;   
            req.request_id = rand(); //initialize request_id to random value
	    req.op_code = opCode;
	    req.op_1 = op1;
	    req.op_2 = op2;	
	}
  	
	//TODO: print out struct content
	
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

