#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <tchar.h>


typedef (WINAPI *TNtQueryInformationProcess)(HANDLE, DWORD, PVOID, DWORD, PVOID);



void DebugObjectCheck(){
	TNtQueryInformationProcess NtQueryInformationProcess;
	HANDLE hDebugObject = NULL;
	NTSTATUS Status;
	
	ULONG result = 0;
	HMODULE hntdll;
	
	hntdll = GetModuleHandle("ntdll.dll");
	NtQueryInformationProcess = (TNtQueryInformationProcess)GetProcAddress(hntdll,
							_T("NtQueryInformationProcess"));
	Status = NtQueryInformationProcess(GetCurrentProcess(), 0x1e, &hDebugObject, 4, NULL);
	printf("before %d, status %x\n", hDebugObject, Status);
	//c0000353이 뜨는디 STATUS_PORT_NOT_SET라고 한다.  
	//An attempt to remove a processes DebugPort was made, but a port was not already associated with the process.
	if(Status != 0x00000000)
	return;
	printf("afcert %d", hDebugObject);
	if (hDebugObject){
		printf("6 NtQueryInformationProcess!\n");
		
	}
}



int main(void) {
	DebugObjectCheck();
	
	return 0;
}
