#include <stdio.h>
int main(){
struct test
{
  unsigned char  field1;
  unsigned short field2;
  unsigned long  field3;
} __attribute__((__packed__));
 
 typedef struct test test_t;
 
struct uptest
{
  unsigned char  field1;
  unsigned short field2;
  unsigned long  field3;
} ;

 typedef struct uptest uptest_t;

 test_t var1;
 uptest_t var2;
 printf("packed size is %d\n",(int) sizeof(var1));
 printf("unpacked size is %d\n",(int) sizeof(var2));
}
