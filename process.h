#ifndef PROCESS_H
#define PROCESS_H

#include <tlhelp32.h>

enum process_status
{
	PROCESS_NOT_OPENED,
	PROCESS_OPENED,
};

struct process_info
{
	DWORD ProcID;
	HANDLE Process;
	process_status Status;
	
	uintptr_t ModuleBaseAddr;
};

DWORD GetProcID(const char* ProcName);

uintptr_t GetModuleBaseAddress(DWORD ProcID, const char* ModuleName);

uintptr_t FindDAMAddr(HANDLE Process, uintptr_t BaseAddr, 
					uint32* Offset, int OffsetCount);


#endif