/*
** server.c -- a stream socket server demo
*/

#include "TCP.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
// void *myThreadFun(void *vargp)
// {
//     sleep(1);
//     printf("Printing GeeksQuiz from Thread \n");
//     return NULL;
// }

// int main()
// {
//     pthread_t thread_id;
//     printf("Before Thread\n");
//     pthread_create(&thread_id, NULL, myThreadFun, NULL);
//     pthread_join(thread_id, NULL);
//     printf("After Thread\n");
//     exit(0);
// }
const uint8_t MASTER_GID = 12;
const uint32_t MAGIC_NUMBER = 0x4A6F7921;
char *portNumber;
struct Node *master;

char *convertLongToIP(uint32_t num)
{
	uint32_t toNetworkIP = htonl(num);
	struct in_addr ip_addr;
	ip_addr.s_addr = num;
	char *ip = inet_ntoa(ip_addr);
	return ip;
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	errno = saved_errno;
}

int max(int x, int y)
{
	if (x > y)
		return x;
	else
		return y;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
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
		column = 0;
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

// adds a slave node to the linked list ring. Master points to this new slave added.
void addSlaveNode(struct Node *master, struct Node *slave)
{
	// so this is going to add a node directly after the master node.
	slave->RID = master->nextRID;
	slave->nextSlaveIP = master->next->IP;
	slave->next = master->next;
	master->nextSlaveIP = slave->IP;
	master->next = slave;
	master->nextRID += 1;

	char* nextIP = convertLongToIP(master->nextSlaveIP);
	printf("Next Slave IP: %s", nextIP);
}

uint8_t getChecksum(uint8_t *message, size_t len)
{

	uint8_t sum = 0;
	int i;
	for (i = 1; i < len; i++)
	{
		uint8_t byte1 = message[i - 1];
		uint8_t byte2 = message[i];

		// sum the two bytes
		sum = byte1 + byte2;

		// if overflow occurs, add one to the sum
		if (sum < byte1)
		{
			sum = sum + 1;
		}
	}

	sum = ~sum;
	return sum;
}

int getPortNumber(uint8_t rid)
{
	return 10010 + (MASTER_GID % 30) * 5 + rid;
}

// this is a tcp connection that waits to receive messages from slaves
// and add the slave nodes to the node ring
void *addSlaveNodeThread(void *vargp)
{
	int sockfd, new_fd, read_value;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	struct message_request request;
	struct message_response response;

	// TCP
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("server: TCP socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					   sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: TCP bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	// printf("Wating to receive message from client...\n");

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	printf("Waiting to receive message from client...\n");

	// sa.sa_handler = sigchld_handler; // reap all dead processes
	// sigemptyset(&sa.sa_mask);
	// sa.sa_flags = SA_RESTART;
	// if (sigaction(SIGCHLD, &sa, NULL) == -1)
	// {
	// 	perror("sigaction");
	// 	exit(1);
	// }

	while (1)
	{
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		sin_size = sizeof their_addr;
		inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr),
				  s, sizeof s);
		printf("server: got connection from %s\n", s);

		/* Listen for response from client */
		read_value = recv(new_fd, &request, sizeof(request), 0);

		if (read_value < 0)
		{
			perror("read from socket");
			exit(1);
		}

		request.magic_number = ntohl(request.magic_number);
		// printf("--------------------------------------------------------\n");
		// printf("Message received:\n");
		// printf("Buf size: %lu \n", sizeof(request));
		// printf("Magic Number: %#04x\n", request.magic_number);
		// printf("GID: %d\n", request.gid);
		// printf("--------------------------------------------------------\n");

		// message validation
		if (sizeof(request) != 5)
		{
			perror("Size of message received is not 5 bytes");
		}
		if (request.magic_number != MAGIC_NUMBER)
		{
			perror("Magic number is not included");
		}

		struct Node *slave = malloc(sizeof(struct Node));
		struct sockaddr_in *get_ip = (struct sockaddr_in *)&their_addr;
		// memcpy(&master->nextSlaveIP, inet_ntoa(get_ip->sin_addr), 4);
		slave->GID = request.gid;
		slave->IP = get_ip->sin_addr.s_addr;
		slave->nextRID = 0;

		addSlaveNode(master, slave);

		//response.gid = MASTER_GID;
		response.gid = request.gid;
		response.magic_number = MAGIC_NUMBER;
		response.nextRID = master->next->RID;
		response.nextSlaveIP = slave->nextSlaveIP;

		// printf("--------------------------------------------------------\n");
		// printf("Message being sent:\n");
		// printf("GID: %d\n", response.gid);
		// printf("Magic Number: %#04x\n", response.magic_number);
		// printf("RID: %d\n", response.nextRID);
		// printf("IP: %s\n", inet_ntoa(get_ip->sin_addr));

		// printf("Message being sent(hex): ");
		// printf("%#04x\\", response.gid);
		// printf("%#04x\\", response.magic_number);
		// printf("%#04x\\", response.nextRID);
		// printf("%#04x\\", response.nextSlaveIP);
		// printf("\n");
		// printf("--------------------------------------------------------\n");

		if (send(new_fd, &response, sizeof(response), 0) == -1)
		{
			perror("send");
			exit(1);
		}
		//exit(0);
		//}
		// printf("Server: Response sent\n");
	}

	close(sockfd);
	return NULL;
}

// UDP server that receives datagrams. If the datagram is for them, displays the datagram,
// if not, sends the datagram to the next Slave IP.
void *handleDatagramThread(void *vargp)
{
	int sockfd, new_fd, read_value;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	struct datagram_message *dgram;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	// set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM; // datagram for UDP
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(NULL, portNumber, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
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
		return NULL;
	}

	freeaddrinfo(servinfo);

	while (1)
	{
		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, &dgram, sizeof(dgram), 0,
								 (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}

		printf("listener: got packet from %s\n",
			   inet_ntop(their_addr.ss_family,
						 get_in_addr((struct sockaddr *)&their_addr),
						 s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);

		// first, check the checksum

		// get datagram in bytes
		size_t dgram_len = sizeof(dgram);
		unsigned char dgram_in_bytes[dgram_len];
		memcpy(dgram_in_bytes, &dgram, dgram_len);
		uint8_t checksum = getChecksum(dgram_in_bytes, dgram_len - 1);
		int isCorrupted = checksum != dgram->checksum ? 1 : 0;

		// if not corrupted, proceed
		if (isCorrupted == 0)
		{
			// if the message is for me, display the message
			if (dgram->rid_dest == master->RID)
			{
				printf("Datagram message: %s", dgram->message);
			}
			else if (dgram->ttl == 0)
			{
				printf("Time to live expired, dropping datagram");
			}
			// otherwise, forward the message to the next slave ip
			else
			{
				// update ttl
				dgram->ttl -= 1;

				// update checksum

				memcpy(dgram_in_bytes, &dgram, dgram_len);
				checksum = getChecksum(dgram_in_bytes, dgram_len - 1);
				dgram->checksum = checksum;

				int sockfd2, new_fd2, read_value2;
				struct addrinfo hints2, *servinfo2, *p2;
				int rv2;
				int numbytes2;

				int nextPortNumber = getPortNumber(master->nextRID);

				uint32_t nextSlaveVal = htonl(master->nextSlaveIP);
				struct in_addr ip_addr;
				ip_addr.s_addr = nextSlaveVal;
				char *ip = inet_ntoa(ip_addr);
				char port[6];
				sprintf(port, "%d", nextSlaveVal);
				printf("\n The string for the num is %s", port);

				// send the updated datagram
				if ((rv2 = getaddrinfo(ip, port, &hints2, &servinfo2)) != 0)
				{
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					return NULL;
				}

				// loop through all the results and bind to the first we can
				for (p2 = servinfo2; p2 != NULL; p2 = p2->ai_next)
				{
					if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
										  p2->ai_protocol)) == -1)
					{
						perror("listener: socket");
						continue;
					}

					if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1)
					{
						close(sockfd2);
						perror("listener: bind");
						continue;
					}
					break;
				}

				if (p2 == NULL)
				{
					fprintf(stderr, "listener: failed to bind socket\n");
					return NULL;
				}

				freeaddrinfo(servinfo2);

				printf("sending message...\n");

				if ((numbytes2 = sendto(sockfd2, &dgram, sizeof(dgram), 0,
										p2->ai_addr, p2->ai_addrlen)) == -1)
				{
					perror("talker: sendto");
					exit(1);
				}

				close(sockfd2);
			}
		}
	}

	close(sockfd);

	return NULL;
}

// sends a message to the node ring
void *sendMessageThread(void *vargp)
{
	sleep(2);

	int sockfd, new_fd, read_value;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	struct datagram_message dgram;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	unsigned char rid;
	char message_to_send[64];

	printf("\n");

	// repeatedly prompt the user for input
	while (1)
	{

		dgram.gid = MASTER_GID;
		dgram.magic_number = MAGIC_NUMBER;
		dgram.ttl = (uint8_t)255;
		dgram.rid_src = master->RID;

		printf("Please enter RID you want to send to: ");
		scanf("%hhu", &rid);
		printf("Please enter message you want to send: ");
		scanf("%s", message_to_send);

		dgram.rid_dest = rid;
		dgram.message = message_to_send;

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;	// set to AF_INET to force IPv4
		hints.ai_socktype = SOCK_DGRAM; // datagram for UDP
		hints.ai_flags = AI_PASSIVE;	// use my IP

		// uint32_t nextSlaveVal = htonl(master->nextSlaveIP);
		// struct in_addr ip_addr;
		// ip_addr.s_addr = nextSlaveVal;
		char *ip = convertLongToIP(master->nextSlaveIP);
		printf("Next IP %s\n", ip);

		// memcpy(nextSlaveIP, (char *)&nextSlaveVal, sizeof(nextSlaveVal));

		// if ((rv = getaddrinfo(ip, portNumber, &hints, &servinfo)) != 0)
		// {
		// 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		// 	return NULL;
		// }

		// // loop through all the results and bind to the first we can
		// for (p = servinfo; p != NULL; p = p->ai_next)
		// {
		// 	if ((sockfd = socket(p->ai_family, p->ai_socktype,
		// 						 p->ai_protocol)) == -1)
		// 	{
		// 		perror("listener: socket");
		// 		continue;
		// 	}

		// 	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		// 	{
		// 		close(sockfd);
		// 		perror("listener: bind");
		// 		continue;
		// 	}
		// 	break;
		// }

		// if (p == NULL)
		// {
		// 	fprintf(stderr, "listener: failed to bind socket\n");
		// 	return NULL;
		// }

		// freeaddrinfo(servinfo);

		// printf("sending message...\n");

		// if ((numbytes = sendto(sockfd, &dgram, sizeof(dgram), 0,
		// 					   p->ai_addr, p->ai_addrlen)) == -1)
		// {
		// 	perror("talker: sendto");
		// 	exit(1);
		// }

		// close(sockfd);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{ //error entering in command line prompt: client servername
		fprintf(stderr, "usage: server portNumber\n");
		exit(1);
	}

	portNumber = argv[1];

	// initializing the master node of the linked list.
	master = malloc(sizeof(struct Node));
	master->GID = MASTER_GID;
	master->IP = MASTER_IP;
	master->RID = 0;
	master->nextRID = 1;
	master->next = master;
	master->nextSlaveIP = master->next->IP;

	// you're going to create three seperate threads.
	// 1. A TCP thread that adds slave nodes to the ring and sends back messages to the slave (Lab 2)
	// 2. A UDP thread that receives datagrams and displays the message if its for it or forwards the datagram if not
	// 3. A prompt that asks the user for a RID and a message m to send. This will be forwarded to the ring.

	pthread_t addSlaveThreadID;
	pthread_t handleDatagramID;
	pthread_t sendMessageID;

	pthread_create(&addSlaveThreadID, NULL, addSlaveNodeThread, NULL);
	pthread_create(&handleDatagramID, NULL, handleDatagramThread, NULL);
	pthread_create(&sendMessageID, NULL, sendMessageThread, NULL);

	pthread_join(addSlaveThreadID, NULL);
	pthread_join(handleDatagramID, NULL);
	pthread_join(sendMessageID, NULL);

	return 0;
}
