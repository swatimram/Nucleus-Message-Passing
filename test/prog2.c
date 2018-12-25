#include "../userprog/syscall.h"


int main()
{
    
char *sender = "../test/prog1";
char *message ;
char *answer= "Answer from prog2 to prog1";
int bufferID = -1;
int result = -1;
    bufferID = WaitMessage(sender, message, bufferID);
    result = SendAnswer(result, answer,bufferID);
    Halt();
    return 0;

}
