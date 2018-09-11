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

typedef struct message_request //TODO: move to a .h file
{
  unsigned int  total_message_length:8;
  unsigned int request_id:8;
  unsigned int op_code:8;
  unsigned int num_operands:8;
  signed int op_1:16; 
  signed int op_2:16;
} message_request;