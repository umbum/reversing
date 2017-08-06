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
            //hinstDLL���� �ڱ��ڽ��� module�� ���´�.
            //DLL�� HINSTANSE�� HMODULE�� ����.
            break;

    }
    return TRUE; // succesful
}

LRESULT CALLBACK KeyboardProc (int nCode, WPARAM wParam, LPARAM lParam){
    char szPath[MAX_PATH] = {0};
    char *p = NULL;

    if(nCode >= 0){
        //key press�̸� lParam�� 31bit�� 0�̹Ƿ�.
        if( !(lParam & 0x80000000)){
            GetModuleFileName(NULL, szPath, MAX_PATH);
            p = strrchr(szPath, '\\');

            //���� PROCESS �̸� 1 ����
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
SetWindowsHookEx�� �̿��ϸ� � ���μ������� �ش� �޽�����
�߻����� ��, OS�� �� �Լ��� ���ڷ� �ѱ� dll�� �޽����� �߻���
���μ����� ������ injection��Ű��
��ϵ� hook procedure�� ȣ���Ѵ�
�� �ڵ����� �ش� ���μ����� remote LoadLibrary���ش�.
*/

DLL_EXPORT void HookStop(){
    if(g_hHook){
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}
