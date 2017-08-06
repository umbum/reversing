#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

#define DLL_PATH ("C:\\Users\\Administrator\\Desktop\\CB\\CBBUM\\work\\UserDll.dll")
#define PROC_NAME ("notepad.exe")

BOOL InjectDll(DWORD pid, LPCSTR dll);
DWORD FindProcessID(LPCSTR szProcessName);

int main()
{
    DWORD dwPID = 0xFFFFFFFF;

    dwPID = FindProcessID(PROC_NAME);
    if(dwPID == 0xFFFFFFFF){
        printf("There is no \"%s\" process\n", PROC_NAME);
        system("pause");
        return 1;
    }

    if(!InjectDll(dwPID, DLL_PATH))
        printf("FAIL\n");
    printf("INJECT\n");
    system("pause");
    return 0;
}

BOOL InjectDll(DWORD pid, LPCSTR dllPath){
    HANDLE hProcess, hThread;
    LPVOID targetAddr;
    LPTHREAD_START_ROUTINE LoadLibAddr;


    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(hProcess == NULL){
        return false;
    }

    targetAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath)+1, MEM_COMMIT, PAGE_READWRITE);
    if(targetAddr){
        printf("targetAddr is %p\n", targetAddr);
        WriteProcessMemory(hProcess, targetAddr, dllPath, strlen(dllPath), NULL );
        LoadLibAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

        hThread = CreateRemoteThread(hProcess, NULL, 0, LoadLibAddr, targetAddr, 0, NULL);
        if(hThread){
            printf("Create Thread\n");
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

        VirtualFreeEx(hProcess, targetAddr, 0, MEM_RELEASE);
    }

    CloseHandle(hProcess);

    return true;
}



DWORD FindProcessID(LPCSTR szProcessName){

    DWORD dwPID = 0xFFFFFFFF;
    HANDLE hSnapShot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

    Process32First(hSnapShot, &pe);
    do{
        if(!_stricmp(szProcessName, pe.szExeFile)){
            dwPID = pe.th32ProcessID;
            break;
        }
    }while(Process32Next (hSnapShot, &pe));

    CloseHandle(hSnapShot);

    return dwPID;
}
