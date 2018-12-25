#include "syscall.h"

int main()
{
     char *sender = "../test/prog7";
     char *answer = "Answer from Prog8";
     char *messagePtr;
     int bufferID = -1;
     int result = -1;
     bufferID =  WaitMessage(sender,messagePtr,bufferID);

  return 0;
}
