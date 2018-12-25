#include "syscall.h"


int main()
{
     char *receiver = "../test/prog8";
     char *message = "Hello from Prog7";
     char *answerPtr;
     int bufferID = -1;
     int result = -1;
     bufferID =  SendMessage(receiver,message,bufferID);
     result  =  WaitAnswer(result,answerPtr,bufferID);

  return 0;
}
