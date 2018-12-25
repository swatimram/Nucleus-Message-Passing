#include "messagebuffer.h"
#include "main.h"
MessageBuffer::MessageBuffer(Thread *sender, Thread *receiver)
{
    this->sender = sender;
    this->receiver = receiver;
    this->bufferID = kernel->mBuffers->FindAndSet();
    this->messages = new List<string>();
    isActive = true;
}

MessageBuffer::MessageBuffer(Thread *sender, Thread *receiver, int bufferID)
{
    this->sender = sender;
    this->receiver = receiver;
    this->bufferID = bufferID;
    this->messages = new List<string>();
    isActive = true;
}
