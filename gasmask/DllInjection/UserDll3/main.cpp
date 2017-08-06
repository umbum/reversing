#include "main.h"
#include <windows.h>

#define PATH "c:\\Program Files\\Internet Explorer\\iexplore.exe"
#define URL "www.naver.com"


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            start();

            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

DLL_EXPORT BOOL start(){
    char szCmd[MAX_PATH] = {0};
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    wsprintf(szCmd, "%s %s", PATH, URL);
    if( !CreateProcess(NULL, (LPTSTR)(LPCTSTR)szCmd,
                       NULL, NULL, FALSE,
                       NORMAL_PRIORITY_CLASS,
                       NULL, NULL, &si, &pi))
        return FALSE;

    if(pi.hProcess != NULL)
        CloseHandle(pi.hProcess);

    return TRUE;

}
