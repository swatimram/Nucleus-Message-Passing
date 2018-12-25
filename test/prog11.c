#include "syscall.h"

int main()
{
     char *sender = "../test/prog9";
     char *answer = "Answer from Prog11";
     char *messagePtr;
     int bufferID = -1;
     int result = -1;
     bufferID =  WaitMessage(sender,messagePtr,bufferID);
     result  =  SendAnswer(result,answer,bufferID);
	
  return 0;
}
