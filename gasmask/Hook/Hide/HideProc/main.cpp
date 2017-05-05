#include "Windows.h"
#include <stdio.h>
#include <tlhelp32.h>


//DWORD _EnableNTPrivilege(LPCTSTR szPrivilege, DWORD dwState);
BOOL InjectToAll(int nMode, LPCTSTR szDllPath);
BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath);
BOOL EjectDll(DWORD dwPID, LPCTSTR szDllPath);

typedef void (*PFN_SetProcName)(LPCTSTR szProcName);
//PFN�� �� �ǹ��ϴ°���..

enum {
      INJECTION_MODE = 0,
      EJECTION_MODE //�ڵ����� 1.
};

int main(int argc, char* argv[]){
    int nMode = INJECTION_MODE;
    HMODULE hLib = NULL;
    PFN_SetProcName SetProcName = NULL;

    if (argc != 4){
        printf("\n Usage : HideProc.exe <-hide|-show> <process name> <dll path> \n\n");
        return 1;
    }

   // _EnableNTPrivilege(SE_DEBUG_NAME, SE_PRIVILEGE_ENABLED);

    hLib = LoadLibrary(argv[3]);
    //LoadLibrary�� ���ڷ� DLL�� ����.

    SetProcName = (PFN_SetProcName)GetProcAddress(hLib, "SetProcName");
    SetProcName(argv[2]);
    //DLL�� SetProcName�Լ��� ��ȯ�ް�,
    //�� �Լ��� ���� Process name�� ���ڷ� ����.

    if( !_stricmp(argv[1], "-show") )
        nMode = EJECTION_MODE;

    InjectToAll(nMode, argv[3]);

    FreeLibrary(hLib);

    return 0;

}

/**
The AdjustTokenPrivileges function cannot add
new privileges to the access token.
It can only enable or disable the token's existing privileges.
��µ�??��� ���ѻ���Ѵٴ°���.

DWORD _EnableNTPrivilege(LPCTSTR szPrivilege, DWORD dwState){
    DWORD dwRtn = 0;
    HANDLE hToken;
    LUID luid;
    DWORD cbTP;


    if( OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken)){
        if( LookupPrivilegeValue(NULL, szPrivilege, &luid)){
            //C++�̶� ���⼭ ������
            BYTE t1[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)];
            BYTE t2[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)];
            cbTP = sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES);

            //������ ���� BYTE�迭�� ����Ű�� ������ �� ��.
            PTOKEN_PRIVILEGES pTP = (PTOKEN_PRIVILEGES)t1;
            PTOKEN_PRIVILEGES pPrevTP = (PTOKEN_PRIVILEGES)t2;

            //���������� ���ѻ���� �̷�� ���� �κ� �� ��.
            pTP->PrivilegeCount = 1;
            pTP->Privileges[0].Luid = luid;
            pTP->Privileges[0].Attributes = dwState;

            if( AdjustTokenPrivileges(hToken, FALSE, pTP,
                                        cbTP, pPrevTP, &cbTP))
                    dwRtn = pPrevTP->Privileges[0].Attributes;

        }

        CloseHandle(hToken);

    }

    return dwRtn;
}
*/

BOOL InjectToAll(int nMode, LPCTSTR szDllPath){
    DWORD dwPID = 0;
    HANDLE hSnapShot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe;
    char *p = NULL;
    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

    printf("======LIST=====\n\n");

    Process32First(hSnapShot, &pe);
    do{
        dwPID = pe.th32ProcessID;

        if(dwPID < 100)
            continue;

        if(nMode == INJECTION_MODE){
           printf("%s\n", pe.szExeFile);
            InjectDll(dwPID, szDllPath);
        }
        else{
           printf("%s\n", pe.szExeFile);
            EjectDll(dwPID, szDllPath);
        }

    }while(Process32Next(hSnapShot, &pe));

    CloseHandle(hSnapShot);

    return TRUE;

}


//Eject��°� �ᱹ FreeLibrary��. Inject�� LoadLibrary��.
BOOL EjectDll(DWORD dwPID, LPCTSTR szDllPath){

    BOOL bMore = FALSE;
    BOOL bFound = FALSE;
    HANDLE hSnapShot, hProcess, hThread;
    MODULEENTRY32 me = {    sizeof(me)  };  //dwSize = sizeof(me)
    LPTHREAD_START_ROUTINE pThreadProc;

    if(INVALID_HANDLE_VALUE == (hSnapShot = CreateToolhelp32Snapshot(
                                        TH32CS_SNAPMODULE, dwPID)))
        return FALSE;

    bMore = Module32First(hSnapShot, &me);
    for( ; bMore ; bMore = Module32Next(hSnapShot, &me)){
        if(!_stricmp(me.szModule, szDllPath) ||
            !_stricmp(me.szExePath, szDllPath)){
                bFound = TRUE;
                break;
            }//ã���� break
    }

    if( !bFound){
        CloseHandle(hSnapShot);
        return FALSE;
    }//��ã���� return

    if( !(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID))){
        CloseHandle(hSnapShot);
        return FALSE;
    }

    pThreadProc = (LPTHREAD_START_ROUTINE)
                    GetProcAddress(GetModuleHandle("kernel32.dll"), "FreeLibrary");
    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
    /**
        LoadLibrary�� LPCTSTR FileName�� ���ڷ� �ѱ�� �ݸ�
        FreeLibrary�� HMODULE hModule�� ���ڷ� �ѱ�ٴ� ��!
        �׷��� modBaseAddr�� ����!!
    */
    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProcess);
    CloseHandle(hSnapShot);

    return TRUE;

}

BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath){
    HANDLE hProcess, hThread;
    LPVOID targetAddr;
    LPTHREAD_START_ROUTINE LoadLibAddr;


    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if(hProcess == NULL){
        return false;
    }

    targetAddr = VirtualAllocEx(hProcess, NULL, strlen(szDllPath)+1, MEM_COMMIT, PAGE_READWRITE);
    if(targetAddr){
        WriteProcessMemory(hProcess, targetAddr, szDllPath, strlen(szDllPath), NULL );
        LoadLibAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

        hThread = CreateRemoteThread(hProcess, NULL, 0, LoadLibAddr, targetAddr, 0, NULL);
        if(hThread){
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

        VirtualFreeEx(hProcess, targetAddr, 0, MEM_RELEASE);
    }

    CloseHandle(hProcess);

    return true;

}
