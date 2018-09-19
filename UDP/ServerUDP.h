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

struct message_send
{
  u_int8_t total_message_length;
  u_int8_t request_ID;
  u_int8_t error_code;
  int32_t result;
} __attribute__((__packed__));

struct message_receive
{
  size_t total_message_length;
  unsigned int request_id;
  unsigned int op_code;
  unsigned int num_operands;
  signed int op_1;
  signed int op_2;
} __attribute__((__packed__));

enum opCodes{addOp, subOp, orOp, andOp, shRightOp, shLeftOp, notOp};

// calculates the result to be sent back depending on the opCode
void getResult(int op1, int opCode, int op2, int result);

// sets the op2 value if it's not a NOT operation and sets error_code
// to see it meets total message length requirements and op code requirements
void checkErrors(struct message_receive *buf, int op2, int error_code);

// adds two operands together
void add(int op1, int op2, int result);

// subtracts two operands 
void subtract(int op1, int op2, int result);

// performs bitwise OR on two operands
void OR(int op1, int op2, int result);

// performs bitwise AND on two operands
void AND(int op1, int op2, int result);

// performs right logical shift on two operands
void rShift(int op1, int op2, int result);

// performs left logical shift on two operands
void lShift(int op1, int op2, int result);

// performs bitwise NOT on two operands
void NOT(int op, int result);


