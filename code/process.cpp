#include "process.h"

DWORD GetProcID(const char* ProcName)
{
	DWORD ProcID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 ProcEntry = {};
		ProcEntry.dwSize = sizeof(PROCESSENTRY32);
		
		if(Process32First(hSnap, &ProcEntry))
		{
			do
			{
				if(_stricmp(ProcEntry.szExeFile, ProcName) == 0)
				{
					ProcID = ProcEntry.th32ProcessID;
					break;
				}
			} while(Process32Next(hSnap, &ProcEntry));
		}
	}
	CloseHandle(hSnap);
	return ProcID;
}

uintptr_t GetModuleBaseAddress(DWORD ProcID, const char* ModuleName)
{
	uintptr_t ModuleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcID);
	if(hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry = {};
		ModuleEntry.dwSize = sizeof(MODULEENTRY32);
		
		if(Module32First(hSnap, &ModuleEntry))
		{
			do
			{
				if(_stricmp(ModuleEntry.szModule, ModuleName) == 0)
				{
					ModuleBaseAddress = (uintptr_t)ModuleEntry.modBaseAddr;
					break;
				}
			} while(Module32Next(hSnap, &ModuleEntry));
		}
	}
	
	CloseHandle(hSnap);
	
	return ModuleBaseAddress;
}

uintptr_t FindDAMAddr(HANDLE Process, uintptr_t BaseAddr, 
					uint32* Offset, int OffsetCount)
{
	uintptr_t Result = BaseAddr;
	for(int i = 0; i < OffsetCount; ++i)
	{
		ReadProcessMemory(Process, (void*)Result, &Result, sizeof(Result), 0);
		Result += Offset[i];
	}
	return Result;
}

process_info GetProcessInfo(char* ProcessName, char* ModuleName)
{
	process_info ProcessInfo = {};
	ProcessInfo.ProcID = GetProcID(ProcessName);
	if(ProcessInfo.ProcID)
	{
		ProcessInfo.Process = 
		OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessInfo.ProcID);
		ProcessInfo.Status = PROCESS_OPENED;
		ProcessInfo.ModuleBaseAddr = 
		GetModuleBaseAddress(ProcessInfo.ProcID, ModuleName);
	}
	else
	{
		ProcessInfo.Status = PROCESS_NOT_OPENED;
	}
	return ProcessInfo;
}

void ReadWriteMemory(process_info* Info, button* Button, 
					 game_input* Input, keys Hotkey,
					 uintptr_t ItemPtr, uint32* Value, int n)
{
	if(ReadProcessMemory(Info->Process, (void*)ItemPtr, Value, sizeof(uint32), 0)) 
	{
		if(Button->State == BUTTON_STATE_L_CLICK || 
		   Input->Keyboard.Keys[Hotkey].EndedDown)
		{
			*Value += n;
			WriteProcessMemory(Info->Process, (void*)ItemPtr, 
							   Value, sizeof(uint32), 0);
			
		}
	}
}
