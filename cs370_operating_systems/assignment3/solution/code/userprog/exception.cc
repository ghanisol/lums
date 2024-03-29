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
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

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
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void threadCreator(int arg);

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	
    if (which == SyscallException)
    {
    	switch(type)
	{
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
               	interrupt->Halt();
               	break;
	    case SC_Exit:
		{
		    int status = machine->ReadRegister(4);
		    if (status == 0)
		    {
		        printf("%s", "Program Exited Normally!\n");
		    }
		    else
		    {
		        printf("%s", "Program Didn't Exited Normally!\n");
		    }
		}
		delete currentThread->space;
		currentThread->Finish();
		break;

	    case SC_Create:
	    {	
		int f_buff = machine->ReadRegister(4);
		int len = 150;
		char *f_name = new char[len];

	 	int *c = new int;
		if (!machine->ReadMem(f_buff, 1, c))
		{
		    return;
		}	   

		int i = 0;
		
		while (i < len-1)
		{
		    f_name[i] = *c;
		    i++;
		    f_buff++;
		    if (!machine->ReadMem(f_buff, 1, c))
		    {
			return;
 		    }
                    if((char)*c == '\0')
                    {
                        break;
                    }


		}
		f_name[i] = '\0';

		fileSystem->Create(f_name, 0);

		printf("%s %s", f_name, " File created successfully\n");

		delete [] f_name;

		int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);

		break;
	    }
	    case SC_Open:
	    {
             	int f_buff = machine->ReadRegister(4);
                int len = 150;
                char *f_name = new char[len];

                int *c = new int;
                if (!machine->ReadMem(f_buff, 1, c))
                {
                    return;
                }

                int i = 0;

                while (i < len-1)
                {
                    f_name[i] = *c;
                    i++;
                    f_buff++;
                    if (!machine->ReadMem(f_buff, 1, c))
                    {
                     	return;
                    }
                    if((char)*c == '\0')
                    {
                        break;
                    }


                }
                f_name[i] = '\0';

		OpenFile *f;
		int f_id;

		f = fileSystem->Open(f_name);
		
		printf("%s %s", f_name, " File Opened!\n");
		delete [] f_name;

		if (f)
		{
		    f_id = currentThread->space->fileT->addFile(f);

		}

		machine->WriteRegister(2, f_id);

                int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);

		break;

	    }
	    case SC_Close:
	    {
		int fid = machine->ReadRegister(4);
		
		OpenFile *f = currentThread->space->fileT->getFile(fid);
		
		currentThread->space->fileT->removeFile(fid);
		
		currentThread->space->fileT->printTable();
		
		printf("%s %i\n", "Closing File ", fid);
		
		int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);

		
		break;
		
  	    }
	    case SC_Write:
	    {
		int buff = machine->ReadRegister(4);
		int len = machine->ReadRegister(5);
		int fid = machine->ReadRegister(6);


		char *str = new char[len+1];

                int *c = new int;
                if (!machine->ReadMem(buff, 1, c))
                {
                    return;
                }

                int i = 0;

                while (i < len)
                {
                    str[i] = *c;
                    i++;
                    buff++;
                    if (!machine->ReadMem(buff, 1, c))
                    {
                     	return;
                    }
		}
                
		str[i] = '\0';


		if (fid == ConsoleInput)
		{
		   return;
		}

		if (fid == ConsoleOutput)
		{
		    printf("%s\n", str);
		}
		else
		{
		    OpenFile* f;
		    if ((f = currentThread->space->fileT->getFile(fid)))
		    {
			int flag = f->Write(str, len);
		    }
		    else
		    {
			printf("%s", "Bad OpenFileId passed to Write\n");
			return;
		    }
		}
		
	        int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);

		break;

	    }
	    case SC_Read:
	    {
		int buff = machine->ReadRegister(4);
		int len = machine->ReadRegister(5);
		int fid = machine->ReadRegister(6);
		int len_read = 0;

		if (fid == ConsoleOutput)
		{
		    return;
		}

		char *str = new char[len+1];

		if (fid == ConsoleInput)
		{
               	    fgets(str, len+1, stdin);
		    len_read = strlen(str);
		}
		else
		{
		    OpenFile* f;
		    if (f = currentThread->space->fileT->getFile(fid))
		    {
		  	len_read = f->Read(str, len);
		    }
		    else
		    {
			printf("%s", "Bad OpenFileId passed to Read\n");
		    }
		}

		int i = 0;
		while (i < len+1)
		{
		    if (!machine->WriteMem(buff, 1, (int)str[i]))
		    {
			return;
		    }
		    i++;
		    buff++;
		}
		
		machine->WriteRegister(2, len_read);

		int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);
		
		break;
  	    }
	    case SC_Exec:
	    {
		int f_buff = machine->ReadRegister(4);
	  	int len = 150;
		char *f_name = new char[len];

		int *c = new int;
                if (!machine->ReadMem(f_buff, 1, c))
                {
                    return;
                }
		
		int i = 0;

                while (i < len-1)
                {
                    f_name[i] = *c;
                    i++;
                    f_buff++;
                    if (!machine->ReadMem(f_buff, 1, c))
                    {
                     	return;
                    }
		    if((char)*c == '\0')
		    {
		 	break;
		    }

                }

                f_name[i] = '\0';
		
		printf("\nFile Going to be executed : %s\n", f_name);		
		IntStatus  oldLevel = interrupt->SetLevel(IntOff);

		OpenFile* executable = fileSystem->Open(f_name);
		AddrSpace *space;
		space = new AddrSpace(executable);
		
		(void) interrupt->SetLevel(oldLevel);
		
		Thread *t = new Thread(f_name);
		t->space = space;
		
		int pid = t->getPID();
		
		t->Fork(threadCreator, pid);

		machine->WriteRegister(2, pid);

                int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);
		
		break;
	    }
	    case SC_Join:
	    {
		int s_id = machine->ReadRegister(4);
		int s = 0;
		Thread* t = scheduler->getProcess(s_id);
		if (t)
		{
		    s = t->join();
		}

		machine->WriteRegister(2, s);
		
                int pc;
                pc = machine->ReadRegister(PCReg);
                machine->WriteRegister(PrevPCReg, pc);
                pc = machine->ReadRegister(NextPCReg);
                machine->WriteRegister(PCReg, pc);
                pc += 4;
                machine->WriteRegister(NextPCReg, pc);


		break;
	    }
	    default:
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(FALSE);

	}
    }	
}


void threadCreator(int arg)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->Run();

    ASSERT(FALSE);
}
