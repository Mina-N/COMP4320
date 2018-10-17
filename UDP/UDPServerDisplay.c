/*
** Original  listener.c by Beej -- a datagram sockets "server" demo
** Modified by Saad Biaz 08/29/2014
** UDPServerDisplay
** Modification 1 (_M1): Can bind to a port # provided on the command line
** Modification 2 (_M2): Server must loop indefinitely
** Modification 3 (_M3): Display Datagram as individual bytes in hexadecimal 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// _M1 Not needed anymore #define MYPORT "10010"  // the port users will be connecting to

#define MAXBUFLEN 100

void displayBuffer(char *Buffer, int length); // _M3

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// _M1 , now we need arguments int main(void)
int main(int argc, char *argv[]) // _M1 
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	/* _M1 Begin */
	if (argc != 2) {
		fprintf(stderr,"usage: UDPServerDisplay Port# \n");
		exit(1);
	}
	/* _M1 End*/

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1] /* _M1 MYPORT */, &hints, &servinfo)) != 0) {
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


	while (1){ 	// _M2 
	  printf("\n >>>> listener: waiting for a datagram...\n");

	  addr_len = sizeof their_addr;
	  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
				   (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	    perror("recvfrom");
	    exit(1);
	  }

	  printf("listener: got packet from %s\n",
		 inet_ntop(their_addr.ss_family,
			   get_in_addr((struct sockaddr *)&their_addr),
			   s, sizeof s));
	  printf("listener: packet is %d bytes long\n", numbytes);
	  buf[numbytes] = '\0';
	  printf("listener: packet contains \"%s\"\n", buf);
	  displayBuffer(buf,numbytes); // _M3
	} // _M2 
	close(sockfd);

	return 0;
}

// _M3 Begin
void displayBuffer(char *Buffer, int length){
int currentByte, column;

currentByte = 0;
printf("\n>>>>>>>>>>>> Content in hexadecimal <<<<<<<<<<<\n");
while (currentByte < length){
  printf("%3d: ", currentByte);
  column =0;
  while ((currentByte < length) && (column < 10)){
    printf("%2x ",Buffer[currentByte]);
    column++;
    currentByte++;
  }
  printf("\n");
 }
 printf("\n\n");
}
// _M3 End
