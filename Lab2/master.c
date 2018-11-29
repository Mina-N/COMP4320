/*
** server.c -- a stream socket server demo
*/

#include "TCP.h"
const uint8_t MASTER_GID = 12;
const long MAGIC_NUMBER = 0x4A6F7921;

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
}

int main(int argc, char *argv[])
{
	int sockfd, udpfd, new_fd, maxfd, nready, num_bytes, read_value; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	struct message_request buf;
	struct message_request_udp udp_buf;
	struct message_response response;
	struct message_response_udp udp_response;
	pid_t childpid;
	fd_set rset;
	char *message_buf;
	unsigned char msg_sent[10];
	char message[MAXDATASIZE];
	char nextSlaveIP[15];
	char *portNumber;
	char buffer[1024];
	ssize_t n;
	socklen_t len;
	const int on = 1;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t addr_len;

	// initializing the master node of the linked list.
	struct Node *master = malloc(sizeof(struct Node));
	master->GID = MASTER_GID;
	master->IP = MASTER_IP;
	master->RID = 0;
	master->nextRID = 1;
	master->next = master;
	master->nextSlaveIP = master->next->IP;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if (argc != 2)
	{ //error entering in command line prompt: client servername
		fprintf(stderr, "usage: server portNumber\n");
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

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}

	////////////////////////////////////////////////////////////
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
		if ((udpfd = socket(p->ai_family, p->ai_socktype,
							p->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}

		if (bind(udpfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(udpfd);
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

	/* create UDP socket */
	//udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	// binding server addr structure to udp sockfd
	//bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	// clear the descriptor set
	FD_ZERO(&rset);

	maxfd = max(sockfd, udpfd) + 1;

	while (1)
	{

		// set sockfd and udpfd in readset
		FD_SET(sockfd, &rset);
		FD_SET(udpfd, &rset);

		// select the ready descriptor
		nready = select(maxfd, &rset, NULL, NULL, NULL);

		// if tcp socket is readable then handle
		// it by accepting the connection
		if (FD_ISSET(sockfd, &rset))
		{
			printf("TCP SOCKET\n");
			new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1)
			{
				perror("accept");
				continue;
			}
			if ((childpid = fork()) == 0)
			{
				//////////////////////////////////
				sin_size = sizeof their_addr;
				inet_ntop(their_addr.ss_family,
						  get_in_addr((struct sockaddr *)&their_addr),
						  s, sizeof s);
				printf("server: got connection from %s\n", s);

				/* Listen for response from client */
				read_value = recv(new_fd, &buf, MAXDATASIZE - 1, 0);

				if (read_value < 0)
				{
					perror("read from socket");
					exit(1);
				}

				buf.magic_number = ntohl(buf.magic_number);
				printf("--------------------------------------------------------\n");
				printf("Message received:\n");
				printf("Buf size: %lu \n", sizeof(buf));
				printf("Magic Number: %#04x\n", buf.magic_number);
				printf("GID: %d\n", buf.gid);
				printf("--------------------------------------------------------\n");

				// message validation
				if (sizeof(buf) != 5)
				{
					perror("Size of message received is not 5 bytes");
				}
				if (buf.magic_number != MAGIC_NUMBER)
				{
					perror("Magic number is not included");
				}

				struct Node *slave = malloc(sizeof(struct Node));
				struct sockaddr_in *get_ip = (struct sockaddr_in *)&their_addr;
				// memcpy(&master->nextSlaveIP, inet_ntoa(get_ip->sin_addr), 4);
				slave->GID = buf.gid;
				slave->IP = get_ip->sin_addr.s_addr;
				slave->nextRID = 0;

				addSlaveNode(master, slave);

				//response.gid = MASTER_GID;
				response.gid = buf.gid;
				response.magic_number = MAGIC_NUMBER;
				response.nextRID = master->next->RID;
				response.nextSlaveIP = slave->nextSlaveIP;

				printf("--------------------------------------------------------\n");
				printf("Message being sent:\n");
				printf("GID: %d\n", response.gid);
				printf("Magic Number: %#04x\n", response.magic_number);
				printf("RID: %d\n", response.nextRID);
				printf("IP: %s\n", inet_ntoa(get_ip->sin_addr));

				printf("Message being sent(hex): ");
				printf("%#04x\\", response.gid);
				printf("%#04x\\", response.magic_number);
				printf("%#04x\\", response.nextRID);
				printf("%#04x\\", response.nextSlaveIP);
				printf("\n");
				printf("--------------------------------------------------------\n");

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
		}

		// if udp socket is readable receive the message.

		if (FD_ISSET(udpfd, &rset))
		{
			int rid;
			char m[] = "";
			printf("UDP SOCKET\n");
			printf("Please enter the Ring ID\n");
			scanf("%d", &rid);
			printf("Please enter the Message\n");
			scanf("%s", m);

			addr_len = sizeof their_addr;
			if ((num_bytes = recvfrom(sockfd, buffer, 1000, 0,
									  (struct sockaddr *)&their_addr, &addr_len)) == -1)
			{
				perror("recvfrom");
				// close(sockfd);
				exit(1);
			}

			printf("listener: got message from %s\n",
				   inet_ntop(their_addr.ss_family,
							 get_in_addr((struct sockaddr *)&their_addr),
							 s, sizeof s));

			printf("listener: message is %d bytes long\n", num_bytes);
			buffer[num_bytes] = '\0';

			printf("listener: received message (in hex) contains: ");

			int i = 0;

			while (i < 8)
			{
				printf("%#04x\\", buffer[i]);
				i += 1;
			}

			printf("\n");

			// set message to message_receive struct
			udp_buf.gid = buffer[0];
			udp_buf.magic_number = buffer[1];
			udp_buf.ttl = buffer[2];
			udp_buf.rid_dest = buffer[3];
			udp_buf.rid_src = buffer[4];
			//memcpy(buffer[6], &udp_buf.message, 1);

			/*
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

						printf("Result: %d\n", result);

						message.total_message_length = RESPONSE_BYTES;
						message.result = htonl(result);
						message.request_ID = buf.request_id;
						message.error_code = error_code;

						memcpy(responseArr, &message.total_message_length, 1);
						memcpy(responseArr + 1, &message.request_ID, 1);
						memcpy(responseArr + 2, &message.error_code, 1);
						memcpy(responseArr + 3, &message.result, 4);

						printf("Message being sent (in hex): ");
						int j = 0;
						while(j < 7) {
							printf("%#04x\\", responseArr[j]);
							j++;
						}
						printf("\n");

						if (sendto(sockfd, responseArr, sizeof(responseArr), 0,
								   (const struct sockaddr *)&their_addr, addr_len) == -1)
						{
							perror("sendto");
							exit(1);
						}
						*/
		}
	}
	close(sockfd); // parent doesn't need this
	//exit(0);
	return 0;
}
