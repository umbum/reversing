#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <string.h>


DWORD GetPidFromProcessName(LPTSTR szProcess){
	DWORD dwPid = 0;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD dwLen = strlen(szProcess);
	
	char pnamelower[20];
	printf("%s\n--------\n", szProcess);
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return 0;
	
	pe32.dwSize = sizeof(PROCESSENTRY32);
	
	if(Process32First(hProcessSnap, &pe32)){
		do{
		//	strcpy(pnamelower, pe32.szExeFile); //strcpy가 pe32에 접근하는 횟수때문에 
			strlwr(pe32.szExeFile);				//pointer가 이동하는건지 뭔지모르겠네 
			printf("%s\n", pe32.szExeFile);
			if(strncmp(szProcess, pe32.szExeFile, dwLen) ==0){
				dwPid = pe32.th32ProcessID;

				break;
			}
		}while(Process32Next(hProcessSnap, &pe32));
	}
	
	CloseHandle(hProcessSnap);
	
	return dwPid;
	
}


int main(int argc, char** argv) {

	
	if(GetPidFromProcessName("ollydbg.exe"))
		printf("\n------------\nDBG detected!!\n");
	else
		printf("\n------------\nDBG is not detected!!\n");
	
	system("pause");
	return 0;
}
