#include <iostream>
#include <windows.h>

DWORD TimerCheck(int a, int b){
	DWORD dwStart = GetTickCount();
	int c = a ^ 0x369;
	int d = c + b*0xdead;
	int e = d / a;
	DWORD dwEnd = GetTickCount();
	
	if(dwEnd - dwStart > 1000)
		e = 0;
		
	return e;
}

int main(int argc, char** argv) {
	puts("putputputputuu");
	TimerCheck(3, 2);
	return 0;
}
