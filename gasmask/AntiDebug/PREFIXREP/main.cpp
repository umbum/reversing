#include <stdio.h>
#include <windows.h>

bool IsDbgPresentPrefixCheck(){
    __try{
        __asm __emit 0xF3
        __asm __emit 0x64
        __asm __emit 0xF1
    }__except(EXCEPTION_EXECUTE_HANDLER){
        return false;
    }

    return true;
}

int main()
{
    if(IsDbgPresentPrefixCheck()){
        printf("Dbg");
        return 0;
    }
    printf("not Dbg");


    return 0;
}
