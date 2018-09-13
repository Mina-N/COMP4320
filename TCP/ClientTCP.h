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
  u_int8_t total_message_length:8;
  u_int8_t request_id:8;
  u_int8_t op_code:8;
  u_int8_t num_operands:8;
  int16_t op_1:16; 
  int16_t op_2:16;
} message_request;