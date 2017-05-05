#include <iostream>
#include <windows.h>
#include <excpt.h>

bool CheckOutputDebugStringU(){
	DWORD dwRet = 1;
	char *szString = "window31";
	__asm{
        push szString
        call ds:[OutputDebugString]
        mov dwRet, eax
        }

	if(dwRet == 0)
	return false;
	else
	return true;
}



int main(void) {
	OutputDebugString("Å©Å©ÄÉ");
	if(CheckOutputDebugStringU())
		printf("true = debugging");
	else
		printf("false = nomal");

	return 0;
}
