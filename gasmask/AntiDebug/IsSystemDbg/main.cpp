#include <iostream>
#include <windows.h>


BOOL IsRemoteDebugger(){
    BOOL bDetection = FALSE;
    kdRefreshDebuggerNotPresent();
    //이 함수가 전역변수 KD_DEBUGGER_NOT_PRESENT에 값을 넣어준다.

    if(KD_DEBUGGER_NOT_PRESENT == FALSE){
        bDetection = TRUE;
        DbgPrint("Dbg\n");
    }
    else{
        DbgPrint("Not Dbg\n");
    }

    return bDetection;
}

int main()
{
    IsRemoteDebugger();
    return 0;
}
