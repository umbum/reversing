#include "main.h"
#include "windows.h"

#define IE "c:\\Program Files\\Internet Explorer\\iexplorer.exe"
#define URL "www.naver.com"
#define DST_PROC "notepad.exe"


//AppInit_Dlls라는 레지스트리가 있다는 것만 하고 넘어가자
//VM에서는 안되는듯...
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    char szCmd[MAX_PATH] = {0};
    char szPath[MAX_PATH] = {0};
    char *szProcess = NULL;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            if( !GetModuleFileName(NULL, szPath, MAX_PATH))
                break;
            //이 dll을 로딩한 프로세스 Path를 받아온다
            if( !(szProcess = strrchr(szPath, '\\')))
                break;
            szProcess++;
            /*아마도 \\notepad.exe가 들어오지않을까.
                끝에서부터 \\까지 자르는 함수.
                그래서 ++해주는듯
            */

            if( _stricmp(szProcess, DST_PROC))
                break;

            wsprintf(szCmd, "%s %s", IE, URL);
            if( !CreateProcess(NULL, (LPTSTR)szCmd,
                               NULL, NULL, FALSE,
                               NORMAL_PRIORITY_CLASS,
                               NULL, NULL, &si, &pi))
                break;

            if(pi.hProcess != NULL)
                CloseHandle(pi.hProcess);

            break;

    }
    return TRUE;
}
