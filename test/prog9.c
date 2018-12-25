#include "syscall.h"

int main()
{

     char *receiver1 = "../test/prog10";
     char *receiver2 = "../test/prog11";
     char *message1 = "Message1 from Prog9";
     char *message2 = "Message2 from Prog9";
     char *answerPtr1;
     char *answerPtr2;
     int bufferID1 = -1;
     int bufferID2 = -1;
     int result1 = -1;
     int result2 = -1;
     bufferID1 =  SendMessage(receiver1,message1,bufferID1);
     bufferID2 =  SendMessage(receiver2,message2,bufferID2);

     result1  =  WaitAnswer(result1,answerPtr1,bufferID1);
     result2  =  WaitAnswer(result2,answerPtr2,bufferID2);
	Halt();
  return 0;
}
