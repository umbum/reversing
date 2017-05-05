#include <iostream>
#include <Windows.h>
#include <winternl.h>

typedef unsigned int UNIT

bool CheckParentProcessId(){
	DWORD dwExplorerPid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &dwExplorerPid);
	
	typedef NTSTATUS (WINAPI *TNtQueryInformationProcess)(HANDLE, UNIT, PVOID, ULONG, PULONG);
	TNtQueryInformationProcess fnNtQueryInformationProcess;
	
	NTSTATUS Status = 0;
	PROCESS_BASIC_INFORMATION pbi;
	ZeroMemory(&pbi, sizeof(PROCESS_BASIC_INFORMATION));
	
	fnNtQueryInformationProcess = 
		(TNtQueryInformationProcess)GetProcAddress(GetModuleHandle("ntdll.dll"),
													"NtQueryInformationProcess");
													
	if(fnNtQueryInformationProcess == 0){
		return false;
	}
	
	Status = fnNtQueryInformationProcess(GetCurrentProcess(), 0, (void*)&pbi, sizeof(PROCESS_BASIC_INFORMATION), 0);
	
	if(Status != 0x00000000)
		return false;
	
	if(pbi.ParentProcessId != dwExplorerPid)
		return false;
	
	return true;
}

int main(int argc, char** argv) {
	return 0;
}
