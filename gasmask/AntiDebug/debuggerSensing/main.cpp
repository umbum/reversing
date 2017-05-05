#include <iostream>
#include <windows.h>
#include <excpt.h>


LONG WINAPI UnhandleExcepFilter(PEXCEPTION_POINTERS pExcepPointers){
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)pExcepPointers->ContextRecord->Eax);
	
	//Skip the exception code
	pExcepPointers->ContextRecord->Eip +=2;
	
	return EXCEPTION_CONTINUE_EXECUTION;
	
}
void UnhandleException(){
	SetUnhandledExceptionFilter(UnhandleExcepFilter);
	__asm ("xor %eax, %eax");
	__asm ("div %eax");
	
}

int main(void) {
	printf("kkka\n");
	UnhandleException();
	printf("after");
	return 0;
}
