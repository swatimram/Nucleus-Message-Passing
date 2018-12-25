#include "../userprog/syscall.h"


int main()
{
    char *receiver = "../test/prog2";
    char *message = "Message from prog1 to prog2";
    char *answer;
    int bufferID = -1;
    int result = -1;
    bufferID = SendMessage(receiver, message, bufferID);
    result = WaitAnswer(result, answer,bufferID);
    Halt();
    return 0;

}
