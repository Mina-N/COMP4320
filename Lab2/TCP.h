
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
 uint8_t gid1;
 long magic_number1;
} __attribute__((__packed__));

struct message_response
{
  uint8_t total_message_length;
  uint8_t request_id;
  uint8_t error_code;
  long result;
} __attribute__((__packed__));

struct master_properties {
 uint8_t nextRID;
 uint8_t gid;
 long magic_number;
 uint8_t ring_id;
 long nextSlaveIP;
 /* Declare master variables */
}__attribute__((__packed__));
