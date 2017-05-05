#include "main.h"
#include "windows.h"

#define IE "c:\\Program Files\\Internet Explorer\\iexplorer.exe"
#define URL "www.naver.com"
#define DST_PROC "notepad.exe"


//AppInit_Dlls��� ������Ʈ���� �ִٴ� �͸� �ϰ� �Ѿ��
//VM������ �ȵǴµ�...
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
            //�� dll�� �ε��� ���μ��� Path�� �޾ƿ´�
            if( !(szProcess = strrchr(szPath, '\\')))
                break;
            szProcess++;
            /*�Ƹ��� \\notepad.exe�� ������������.
                ���������� \\���� �ڸ��� �Լ�.
                �׷��� ++���ִµ�
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
