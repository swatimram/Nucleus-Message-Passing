#include "syscall.h"

int main()
{
    char *receiver = "../test/prog6";
    char *message1 = "Message1 from Prog5";
    char *message2 = "Message2 from Prog5";
    char *answerPtr;
    int bufferID = -1;
    int result1 = -1;
    int result2 = -1;
	bufferID =  SendMessage(receiver,message1,bufferID);
	result1  =  WaitAnswer(result1,answerPtr,bufferID);
     
	bufferID =  SendMessage(receiver,message2,bufferID);
	result2  =  WaitAnswer(result2,answerPtr,bufferID);
        Halt();
  return 0;
}
