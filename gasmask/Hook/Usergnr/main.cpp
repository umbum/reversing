#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define DLL_NAME "keygnr.dll"


typedef void (*PFN_HOOKSTART)();
typedef void (*PFN_HOOKSTOP)();

int main(void){
    HMODULE hDll = NULL;
    PFN_HOOKSTART HookStart = NULL;
    PFN_HOOKSTOP HookStop = NULL;
    char ch = 0;

    hDll = LoadLibrary(DLL_NAME);
    if(hDll){
        HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStart");
        HookStop = (PFN_HOOKSTOP)GetProcAddress(hDll, "HookStop");

        HookStart();
        printf("LoadLibrary success\n");
        printf("press 'q' to quit!\n");
        while( getch() != 'q' );

        FreeLibrary(hDll);
    }
    else{
        printf("LoadLibrary fail.\n");
        printf("press 'q' to quit!\n");
        while( getch() != 'q' );
    }

    return 0;
}
