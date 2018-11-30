#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>

#include <arpa/inet.h>


#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100 // max number of bytes we can get at once

#define REQUEST_BYTES 8

#define RESPONSE_BYTES 7
#define MASTER_PORT 10070

#define MASTER_IP 0x7f000001



/*struct message_request
{
  uint8_t total_message_length;
  uint8_t request_id;
  uint8_t op_code;
  uint8_t num_operands;
  short op_1;
  short op_2;
*/

struct message_request
{
 uint8_t gid;
 uint32_t magic_number;
} __attribute__((__packed__));

struct datagram_message
{
 uint8_t gid;
 uint32_t magic_number;
 uint8_t ttl;
 uint8_t rid_dest;
 uint8_t rid_src;
 char *message;
 uint8_t checksum;
} __attribute__((__packed__));


struct message_response {
 uint8_t gid;
 uint32_t magic_number;
 uint8_t nextRID;
 //uint8_t ring_id;
 uint32_t nextSlaveIP;
 /* Declare master variables */
}__attribute__((__packed__));

struct Node {
  uint8_t RID;
  uint8_t nextRID;
  uint32_t nextSlaveIP;
  uint32_t IP;
  uint8_t GID;
  struct Node *next;
};

void addSlaveNode(struct Node* master, struct Node* slave);
