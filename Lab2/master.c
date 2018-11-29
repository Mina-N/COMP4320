#include "TCP.h"
#include <sys/select.h>

const uint8_t MASTER_GID = 12;
//const uint32_t MAGIC_NUMBER = htonl(0x4A6F7921);
const uint32_t MAGIC_NUMBER = 0x4A6F7921;
//const uint32_t MAGIC_NUMBER = 1248819489;

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

// adds a slave node to the linked list ring. Master points to this new slave added.
void addSlaveNode(struct Node* master, struct Node* slave) {
	// so this is going to add a node directly after the master node.
	slave->RID = master->nextRID;
	slave->nextSlaveIP = master->next->IP;
	slave->next = master->next;
	master->nextSlaveIP = slave->IP;
	master->next = slave;
	master->nextRID += 1;
}

char getChecksum(char* message) {
	size_t message_size = sizeof(message) - 1;
	uint8_t sum = 0;
	// looping through byte array message
	for(int i = 1; i < message_size; i++) {
		// get two bytes
		uint8_t byte1 = message[i-1];
		uint8_t byte2 = message[i];

		// sum the bytes
		sum = byte1 + byte2;

		// overflow has occurred, add one
		if(sum < byte1) {
			sum = sum + 1;
		}
	}

	// negate the final sum
	sum = ~sum;
	return sum;
}

int getPortNumber() {
	10010 + (MASTER_GID % 30)*5;
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
	struct message_response response;
	char *message_buf;
	unsigned char msg_sent[10];
	unsigned char datagramBuffer[MAXDATASIZE];
	char nextSlaveIP[15];
	char *portNumber;
	struct sockaddr my_addr;
	fd_set readfds;
	struct timeval tv;



	//struct sockaddr master_adr;
	struct addrinfo *addr_info;
	char myName[1024];
	myName[1023] = '\0';
	//char master[1024];
	//int masterIP;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE; // use my IP
	hints.ai_socktype = SOCK_STREAM;



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


	//Added this information below
	gethostname(myName, 1024);
	//printf("hostname: %s\n", myName);
	int return_value = getaddrinfo(myName, portNumber, NULL, &addr_info);
	if(return_value != 0)
	{
		printf("addr_info is null");
		printf("%d", return_value);
	}
	my_addr = *(addr_info->ai_addr);
	unsigned long myIPAsInt = ((struct sockaddr_in*)&my_addr)->sin_addr.s_addr;
	unsigned long next_slaveIP = myIPAsInt;
	uint32_t next_slave_IP = ntohl(next_slaveIP);
	//printf("Master IP is: ");
	//printf("%#04x\\", next_slave_IP);

	// initializing the master node of the linked list.
	struct Node* master = malloc(sizeof(struct Node));
	master->GID = MASTER_GID;
	master->IP = next_slaveIP;
	master->RID = 0;
	master->nextRID = 1;
	master->next = master;
	master->nextSlaveIP = master->next->IP;

	printf("server: waiting for connections...\n");
	while(1) {  // main accept() loop
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

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
			exit(1);
		}

		buf.magic_number = ntohl(buf.magic_number);
		printf("--------------------------------------------------------\n");
		printf("Message received:\n");
		printf("Buf size: %lu \n", sizeof(buf));
		printf("GID of Slave: %d\n", buf.gid);
		printf("Magic Number: %#04x\n", buf.magic_number);
		printf("--------------------------------------------------------\n");

		// message validation
		if(sizeof(buf) != 5) {
			perror("Size of message received is not 5 bytes");
		}
		if(buf.magic_number != MAGIC_NUMBER) {
			perror("Magic number is not included");
		}

		struct Node* slave = malloc(sizeof(struct Node));
		struct sockaddr_in *get_ip = (struct sockaddr_in *)&their_addr;
		// memcpy(&master->nextSlaveIP, inet_ntoa(get_ip->sin_addr), 4);
		slave->GID = buf.gid;
		slave->IP = get_ip->sin_addr.s_addr;
		slave->nextRID = 0;

		addSlaveNode(master, slave);


		// if the received datagram's rid is 0, message is for me
		if(master->RID == 0) {
			// display the datagram received
		}	
		else {
			// forward the datagram
		}


		response.gid = MASTER_GID;
		response.magic_number = MAGIC_NUMBER;
		response.nextRID = master->next->RID;
		uint32_t nextSlaveIP = ntohl(slave->nextSlaveIP);
		struct in_addr ip_addr;
		ip_addr.s_addr = ntohl(nextSlaveIP);

		printf("--------------------------------------------------------\n");
		printf("Message being sent:\n");
		printf("GID of Master: %d\n", response.gid);
		printf("Magic Number: %#04x\n", response.magic_number);
		printf("RID: %d\n", response.nextRID);
		printf("Next Slave IP: %s\n", inet_ntoa(ip_addr));

		printf("Message being sent(hex): ");
		printf("%#04x\\", response.gid);
		printf("%#04x\\", response.magic_number);
		printf("%#04x\\", response.nextRID);
		printf("%#04x\\", nextSlaveIP);
		printf("\n");
		printf("--------------------------------------------------------\n");

		//TODO: Check size of message
		/*Send message to the client */

		response.nextSlaveIP = htonl(nextSlaveIP);

		//if (!fork()) { // this is the child process
		//	close(sockfd); // child doesn't need the listener

		if (send(new_fd, &response, sizeof(response), 0) == -1)
		{
			perror("send");
			exit(1);
		}
			//exit(0);
		//}
		printf("Server: Response sent\n");
		printf("Waiting for response from Client...\n");
	}
	close(sockfd);  // parent doesn't need this
	//exit(0);
	return 0;
}
