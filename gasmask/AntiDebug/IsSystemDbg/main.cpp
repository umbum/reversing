#include <iostream>
#include <windows.h>


BOOL IsRemoteDebugger(){
    BOOL bDetection = FALSE;
    kdRefreshDebuggerNotPresent();
    //�� �Լ��� �������� KD_DEBUGGER_NOT_PRESENT�� ���� �־��ش�.

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
