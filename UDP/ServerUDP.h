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

#define MYPORT "10022"	// the port users will be connecting to

#define MAXBUFLEN 100


typedef struct message_response {
    u_int8_t total_message_length:8;
    u_int8_t request_ID:8;
    u_int8_t error_code:8;
    int32_t result:32;

} message_response;

enum opCodes{add, sub, or, and, shRight, shLeft, not};