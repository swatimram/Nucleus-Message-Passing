#include "syscall.h"

int main()
{
    char *sender = "../test/prog5";
    char *answer1 = "Answer1 from Prog6";
    char *answer2 = "Answer2 from Prog6";
    char *messagePtr;
    int bufferID = -1;
    int result1 = -1;
    int result2 = -1;
	bufferID =  WaitMessage(sender,messagePtr,bufferID);
    result1  =  SendAnswer(result1,answer1,bufferID);

	bufferID =  WaitMessage(sender,messagePtr,bufferID);
 	result2  =  SendAnswer(result2,answer2,bufferID);
	Halt();
  return 0;
}
