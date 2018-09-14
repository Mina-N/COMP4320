#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include <arpa/inet.h>

#define PORT "10022" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

struct message_request
{
  size_t total_message_length;
  unsigned int request_id;
  unsigned int op_code;
  unsigned int num_operands;
  signed int op_1;
  signed int op_2;
} __attribute__((__packed__));
