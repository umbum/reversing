
#include "main.h"
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "urlmon.lib")

#define DEF_NAVER_ADDR ("http://www.naver.com/index.html")
#define DEF_INDEX_PATH ("C:\\Users\\Administrator\\Desktop\\CB\\CBBUM\\work\\index.html")




DWORD WINAPI ThreadProc (LPVOID lParam){
	URLDownloadToFile(NULL, DEF_NAVER_ADDR, DEF_INDEX_PATH, 0, NULL);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
	HANDLE hThread = NULL;

	switch ( fdwReason ){
		case DLL_PROCESS_ATTACH :
			hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
			CloseHandle(hThread);
			break;
	}
	return TRUE;
}



