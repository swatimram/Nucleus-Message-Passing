// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "../lib/copyright.h"
#include "../threads/main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "../threads/thread.h"
#include "messagebuffer.h"
#include <vector>
#include "sstream"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

char *DUMMY_MESSAGE = "Dummy_Message";
Thread *sendt ;
void cleanBuffer(int bufferID)
{
	if (bufferID >= 0)
		kernel->clearMessageBuffer(bufferID);
	kernel->currentThread->Finish();
}

void updatePC()
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

string getStrFromPtr(int startLoc)
{
        string str = "";
        
        int val;

        while (true)
        {
                kernel->machine->ReadMem(startLoc, 1, &val);
                if ((char)val != '\0')
                {
                        str += (char)val;
                        startLoc++;
                }
                else
                {
                        break;
                }
        }

        return str;
}

void writeMessageInMem(string message, int startLoc)
{
	while (true)
	{
		if (message != "\0")
		{
			int msg;
			stringstream(message) >> msg;
			kernel->machine->WriteMem(startLoc, 1, msg);
			startLoc++;
		}
		else
			break;
	}
}

MessageBuffer *validateCommunication(int bufferID, Thread *receiverProcess)
{
	MessageBuffer *mBuffer = kernel->mBufferQueue[bufferID]->Front();
	if (mBuffer != NULL)
	{
		if (mBuffer->getReceiver() == receiverProcess)
		{
			return mBuffer;
		}
	}
}

MessageBuffer *fetchProcessBuffer(int bufferID, string sender)
{
	std::map<int, MessageBuffer *> messagesQueue = kernel->currentThread->processMessageQueue;
	if (!messagesQueue.empty())
	{
		if (messagesQueue.find(bufferID) != messagesQueue.end())
		{
			return messagesQueue[bufferID];
		}
		else if (sender != "")
		{
			std::vector<MessageBuffer *> v;
			for (map<int, MessageBuffer *>::iterator it = messagesQueue.begin(); it != messagesQueue.end(); ++it)
			{
				v.push_back(it->second);
			}
			for (vector<MessageBuffer *>::iterator it = v.begin(); it != v.end(); it++)
			{
				if ((*it)->getSender()->getName() == sender)
				{
					return (*it);
				}
			}
		}
	}
	return NULL;
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			cout<<"Shutdown, initiated by user program.\n";
			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		case SC_Exit:
      		{
			 IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
          		kernel->currentThread->Finish();  
          		(void) kernel->interrupt->SetLevel(oldLevel);       
          		return;
		}
          
		case SC_SendMessage:
		{
			cout << "\n----- SendMessage -----\n";
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			int receiverName = kernel->machine->ReadRegister(4);
			int message = kernel->machine->ReadRegister(5);
			int bufferID = kernel->machine->ReadRegister(6);
			string receiver = getStrFromPtr(receiverName);
			string msg = getStrFromPtr(message);

			Thread *t = Thread::getThreadByName(receiver);

			if (t->getName() == receiver)
			{
				if (kernel->currentThread->getMessageCount() <= kernel->getMessageLimit())
				{
					if (bufferID == -1)
					{
						MessageBuffer *newBuffer = new MessageBuffer(kernel->currentThread, t);
						if (newBuffer->getBufferID() == -1)
						{
							std::cout << "Message Buffer Exhausted, cannot proceed with communication. " << std::endl;
							kernel->machine->WriteRegister(2, newBuffer->getBufferID());
							updatePC();
							(void)kernel->interrupt->SetLevel(oldLevel);
							return;
							break;
						}
						std::cout << "\nMessage Buffer : " << newBuffer->getBufferID() << " allocated to processes " << kernel->currentThread->getName() << " and " << receiver << std::endl;
		            kernel->currentThread->buff.insert(std::pair<int, string> (newBuffer->getBufferID(), receiver));

						newBuffer->getMessages()->Append(msg);

						List<MessageBuffer *> *messageBufferList = new List<MessageBuffer *>();
						messageBufferList->Append(newBuffer);
						kernel->mBufferQueue[newBuffer->getBufferID()] = messageBufferList;
						t->processMessageQueue[newBuffer->getBufferID()] = newBuffer;

						//return value to user program
						kernel->machine->WriteRegister(2, newBuffer->getBufferID());

						std::cout << "\nMessage sent to Process " << receiver << " with message : " << msg << " by process " << kernel->currentThread->getName() << std::endl;
					}
					else
					{
						//check for valid sender and receiver pair
						MessageBuffer *mBuffer = validateCommunication(bufferID, t);
						if (mBuffer != NULL)
						{
							mBuffer->getMessages()->Append(msg);
							t->processMessageQueue[bufferID] = mBuffer;

							//return value to user program
							kernel->machine->WriteRegister(2, bufferID);
							std::cout << "\nMessage sent to Process " << receiver << " with message : " << msg << " by process " << kernel->currentThread->getName() << std::endl;
						}
						else
						{
							//Pass some error message
							kernel->machine->WriteRegister(2, -1);
							std::cout << "Security error \nProcess " << kernel->currentThread->getName() << " attempted Communication via incorrect Buffer with ID: " << bufferID << " to communicate with process: " << receiver << std::endl;
						}
					}
				}
				else
				{

					MessageBuffer *mBuffer = validateCommunication(bufferID, t);
					if (mBuffer != NULL)
					{
						mBuffer->getMessages()->Append(DUMMY_MESSAGE);
						t->processMessageQueue[bufferID] = mBuffer;

						//return value to user program
						kernel->machine->WriteRegister(2, bufferID);
					}

					std::cout << "Maximum Messages Limit reached by process: " << kernel->currentThread->getName() << std::endl;
					std::cout << "Dummy Response sent to the process: " << receiver << std::endl;
				}
			}
			else
			{
				//no need to send dummy
				cleanBuffer(bufferID);
				std::cout << "Dummy Response" << std::endl;
			}
			updatePC();
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}

		case SC_WaitMessage:
		{
			cout << "\n----- WaitMessage -----\n";
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			int senderPtr = kernel->machine->ReadRegister(4);
			string sender = getStrFromPtr(senderPtr);
			int messagePtr = kernel->machine->ReadRegister(5);
			int bufferID = kernel->machine->ReadRegister(6);
				
//			while (true)
			{
				if (Thread::getThreadByName(sender) != NULL)
				{
					Thread *sndr = Thread::getThreadByName(sender);
					std::map<int, string>::iterator it;
					for (it = sndr->buff.begin(); it != sndr->buff.end(); ++it)
					{
						if (it->second == kernel->currentThread->getName())
						{
							bufferID = it->first;
							break;
						}
					}
					MessageBuffer *mbuffer = fetchProcessBuffer(bufferID, sender);
					if (mbuffer != NULL && !mbuffer->getMessages()->IsEmpty())
					{
						cout<<"\nBuffer ID is :"<<bufferID<<endl;
						string message = mbuffer->getMessages()->RemoveFront();
//						writeMessageInMem(message, messagePtr);
						kernel->machine->WriteRegister(2, mbuffer->getBufferID());
						std::cout << "\nMessage Received by process " << kernel->currentThread->getName() << " from process " << sender << ":" << message << std::endl;
						kernel->currentThread->mcount++;
						updatePC();
						(void)kernel->interrupt->SetLevel(oldLevel);
						return;
						break;
					}
					else
					{
						kernel->currentThread->Yield();
					}
				}
				else
				{
					MessageBuffer *mbuffer = fetchProcessBuffer(bufferID, sender);
					if (mbuffer != NULL && !mbuffer->getMessages()->IsEmpty())
					{
						string message = mbuffer->getMessages()->RemoveFront();
				//		writeMessageInMem(message, messagePtr);
						kernel->machine->WriteRegister(2, mbuffer->getBufferID());
						std::cout << "\nMessage Received by process " << kernel->currentThread->getName() << " from process " << sender << ":" << message << std::endl;
					kernel->currentThread->mcount++;
					}
					else
					{
						//send dummy message
						cleanBuffer(bufferID);
				//		writeMessageInMem(DUMMY_MESSAGE, messagePtr);
						kernel->machine->WriteRegister(2, -1);
						std::cout << "Process: " << sender << " exited without sending any message to process " << kernel->currentThread->getName() << std::endl;
						std::cout << "Dummy Message sent to process: " << kernel->currentThread->getName() << std::endl;
					}
					updatePC();
					(void)kernel->interrupt->SetLevel(oldLevel);
					return;
					break;
				}
			}

//			updatePC();
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}

		case SC_SendAnswer:
		{
/*			if (kernel->currentThread->mcount == 0)
			{
				kernel->currentThread->Yield();
			}
*/			cout<<"\n----- SendAnswer -----\n";
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

			int resultStatus = kernel->machine->ReadRegister(4);
			int answerPtr = kernel->machine->ReadRegister(5);
			string answer = getStrFromPtr(answerPtr);
			int bufferID = kernel->machine->ReadRegister(6);

			MessageBuffer *mBuffer = kernel->currentThread->processMessageQueue[bufferID];
			if (mBuffer != NULL)
			{
				Thread *receiver = mBuffer->getSender();
				if (receiver != NULL && Thread::IsThreadAlive(receiver->getThreadId()))
				{
					if (receiver->processMessageQueue[bufferID] == NULL)
					{
						MessageBuffer *newBuffer = new MessageBuffer(kernel->currentThread, receiver, bufferID);
						newBuffer->getMessages()->Append(answer);
						kernel->mBufferQueue[bufferID]->Append(newBuffer);
						receiver->processMessageQueue[bufferID] = newBuffer;
					}
					else
					{
						receiver->processMessageQueue[bufferID]->getMessages()->Append(answer);
					}
					kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
					std::cout << "\nAnswer sent to Process " << receiver->getName() << " with message : " << answer << " by process " << kernel->currentThread->getName() << std::endl;
				kernel->currentThread->mcount--;
				}
				else
				{
					cleanBuffer(bufferID);
					//no need to send dummy
					kernel->machine->WriteRegister(2, -1);
					std::cout << "Receiver either null or dead" << std::endl;
				}
			}
//			if(kernel->currentThread->mcount == 0)
//			{
				kernel->currentThread->Yield();
//			}
			updatePC();
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}

		case SC_WaitAnswer:
		{
			
			cout<<"\n----- WaitAnswer -----\n";
//			kernel->currentThread->Yield();
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			int resultStatus = kernel->machine->ReadRegister(4);
			int answerPtr = kernel->machine->ReadRegister(5);
			int bufferID = kernel->machine->ReadRegister(6);
			string sender = kernel->currentThread->getName();	
			MessageBuffer *mbuffer = fetchProcessBuffer(bufferID, sender);
			if(mbuffer == NULL)
			{
				kernel->currentThread->Yield();
			}
			else if(mbuffer->getMessages()->IsEmpty())
			{
				kernel->currentThread->Yield();
			}

			sender = "";
			if (kernel->currentThread->processMessageQueue[bufferID] != NULL)
				sender = kernel->currentThread->processMessageQueue[bufferID]->getSender()->getName();
			else
			{
				sender = kernel->mBufferQueue[bufferID]->Front()->getReceiver()->getName();
			}

//			while (true)
			{
				if (Thread::getThreadByName(sender) != NULL)
				{
					mbuffer = fetchProcessBuffer(bufferID, sender);
					if (mbuffer != NULL && !mbuffer->getMessages()->IsEmpty())
					{
						string message = mbuffer->getMessages()->RemoveFront();
//						writeMessageInMem(message, answerPtr);
						kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
						std::cout << "\nAnswer Received by process " << kernel->currentThread->getName() << " from process " << sender << ":" << message << std::endl << std::endl;
						
					}
					else
					{
						kernel->currentThread->Yield();
					}
				}
				else
				{
					MessageBuffer *mbuffer = fetchProcessBuffer(bufferID, sender);
					if (mbuffer != NULL && !mbuffer->getMessages()->IsEmpty())
					{
						string message = mbuffer->getMessages()->RemoveFront();
//						writeMessageInMem(message, answerPtr);
						kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
						std::cout << "\nAnswer Received by process " << kernel->currentThread->getName() << " from process " << sender << ":" << message << std::endl << std::endl;
					}
					else
					{
						cleanBuffer(bufferID);
						writeMessageInMem(DUMMY_MESSAGE, 0);
						kernel->machine->WriteRegister(2, 0);
						std::cout << "Dummy Response sent to: " << kernel->currentThread->getName() << " , Process " << sender << " Exited with no response" << std::endl;
					}
					break;
				}
			}
			updatePC();
			(void)kernel->interrupt->SetLevel(oldLevel);
			return;
			break;
		}

		default:
			cerr << "Unexpected system call " << type << "\n";
			
			break;
		}
		
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		
		break;
	}
	ASSERTNOTREACHED();
}
