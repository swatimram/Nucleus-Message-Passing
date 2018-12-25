#include "syscall.h"

int main()
{
     char *sender1 = "../test/prog12";
     char *sender2 = "../test/prog14";
     char *answer1 = "Answer to Prog12 from Prog13";
     char *answer2 = "Answer to Prog14 from Prog13";
     char *messagePtr1;
     char *messagePtr2;
     int bufferID1 = -1;
     int bufferID2 = -1;
     int result = -1;
     bufferID1 =  WaitMessage(sender1,messagePtr1,-1);
     result  =  SendAnswer(result,answer1,bufferID1);
     bufferID2 =  WaitMessage(sender2,messagePtr2,-1);
     result  =  SendAnswer(result,answer2,bufferID2);
  return 0;
}
