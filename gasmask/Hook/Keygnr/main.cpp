#include "main.h"
#include <stdio.h>
#include <windows.h>

#define PROCESS_NAME "notepad.exe"

HINSTANCE g_hInst = NULL;
HHOOK g_hHook = NULL;
HWND g_Hwnd = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInst = hinstDLL;
            //hinstDLL에는 자기자신의 module이 들어온다.
            //DLL의 HINSTANSE는 HMODULE과 같다.
            break;

    }
    return TRUE; // succesful
}

LRESULT CALLBACK KeyboardProc (int nCode, WPARAM wParam, LPARAM lParam){
    char szPath[MAX_PATH] = {0};
    char *p = NULL;

    if(nCode >= 0){
        //key press이면 lParam의 31bit가 0이므로.
        if( !(lParam & 0x80000000)){
            GetModuleFileName(NULL, szPath, MAX_PATH);
            p = strrchr(szPath, '\\');

            //지정 PROCESS 이면 1 리턴
            if( !_stricmp(p + 1, PROCESS_NAME))
                return 1;
        }

    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

DLL_EXPORT void HookStart(){
    g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInst, 0);
}
/*
SetWindowsHookEx를 이용하면 어떤 프로세스에서 해당 메시지가
발생했을 때, OS가 이 함수에 인자로 넘긴 dll을 메시지가 발생한
프로세스에 강제로 injection시키고
등록된 hook procedure를 호출한다
즉 자동으로 해당 프로세스에 remote LoadLibrary해준다.
*/

DLL_EXPORT void HookStop(){
    if(g_hHook){
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}
