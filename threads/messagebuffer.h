#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#pragma once
#include "thread.h"
#include "kernel.h"
#include "scheduler.h"
class Scheduler;
class Kernel;
class Thread;
class MessageBuffer
{
  private:
    Thread *sender;
    Thread *receiver;
    int bufferID;
    bool isActive;
    List<string> *messages;

  public:
    MessageBuffer(Thread *sender, Thread *receiver);

    MessageBuffer(Thread *sender, Thread *receiver, int bufferID);

    Thread* getSender()
    {
        return this->sender;
    }

    Thread* getReceiver()
    {
        return this->receiver;
    }

    int getBufferID()
    {
        return this->bufferID;
    }

    List<string>* getMessages()
    {
        return messages;
    }

    bool IsActive()
    {
        return this->isActive;
    }

    void setActiveFlag(bool flag)
    {
        this->isActive = flag;
    }
};

#endif
