/*
** server.c -- a stream socket server demo
*/

#include "TCP.h"

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
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
			printf("%2x ",Buffer[currentByte]);
			column++;
			currentByte++;
		}
		printf("\n");
	}
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd, num_bytes, read_value;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	struct message_request buf;
	struct message_response res;
	struct master_properties master;
	unsigned char msg_sent[10];
	char message[MAXDATASIZE];
	char nextSlaveIP[15];
	char *portNumber;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP



	if (argc != 2) { //error entering in command line prompt: client servername
	    fprintf(stderr,"usage: server portNumber\n");
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
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		/* Listen for response from client */
		read_value = recv(new_fd, &buf, MAXDATASIZE-1, 0);
		if (read_value < 0)
		{
			perror("read from socket");
		}



		master.gid = 0;
		master.magic_number = 0x4A6F7921;
		master.ring_id = 12;
		master.nextSlaveIP = 0x4A6F7921;

		memcpy(msg_sent, &master.gid, 1);
		memcpy(msg_sent + 1, &master.magic_number, 4);
		memcpy(msg_sent + 2, &master.ring_id, 1);
		memcpy(msg_sent + 3, &master.nextSlaveIP, 4);

		printf("Message being sent(hex): ");
		int j = 0;
		while(j < 4) {
			printf("%#04x\\", msg_sent[j]);
			j++;
		}
		printf("\n");

		char *message_ptr = message;
		displayBuffer(message_ptr, 2);

		//TODO: Check size of message
		/*Send message to the client */
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, &msg_sent, sizeof(msg_sent), 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		printf("server: Response sent\n");
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
