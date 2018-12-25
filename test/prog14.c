#include "syscall.h"

int main()
{
  char *receiver1 = "../test/prog13";
     char *message1 = "Message to Prog13 from Prog14";
     char *answerPtr1;
     int bufferID1 = -1;
     int result1 = -1;
     bufferID1 =  SendMessage(receiver1,message1,bufferID1);
        
        result1  =  WaitAnswer(result1,answerPtr1,bufferID1);
  return 0;
}
