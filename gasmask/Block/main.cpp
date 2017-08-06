#include <iostream>
#include <windows.h>
#include <WinBase.h>

void BlockAllControl(){
	typedef BOOL (__stdcall *TBLOCKINPUT)(BOOL);
	TBLOCKINPUT fnBlockInput = (TBLOCKINPUT)GetProcAddress(GetModuleHandle("user32.dll"),
													"BlockInput");
	printf("freeze for 5 sec\n");
	fnBlockInput(TRUE);
	
	Sleep(5000);
	
	fnBlockInput(FALSE);
	printf("relieve\n");
}

int main(int argc, char** argv) {
	BlockAllControl();
	return 0;
}
