#include <windows.h>
#include <ail/string.hpp>
#include <ail/types.hpp>
#include "utility.hpp"
#include "patch.hpp"
#include "arguments.hpp"

namespace
{
	void * ZwContinue_address;
	void * KiUserExceptionDispatcher_address;

	unsigned find_window_page;
}

unsigned custom_KiUserExceptionDispatcher_body(EXCEPTION_RECORD * exception_record_pointer, CONTEXT * thread_context_pointer)
{
	CONTEXT & thread_context = *thread_context_pointer;
	EXCEPTION_RECORD & exception_record = *exception_record_pointer;

	write_line("Exception type " + ail::hex_string_32(exception_record.ExceptionCode) + " at " + ail::hex_string_32(thread_context.Eip));

	if(exception_record.ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
		thread_context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
		thread_context.Dr0 = 0;
		thread_context.Dr1 = 0;
		thread_context.Dr2 = 0;
		thread_context.Dr3 = 0;
		thread_context.Dr6 = 0;
		thread_context.Dr7 = 0;
	}

	error("Exception");

	return 0;
}

void it_returned(unsigned return_value)
{
	error("ZwContinue returned " + ail::hex_string_32(return_value));
}


//this version works only for Vista SP2 I suspect

void __declspec(naked) custom_KiUserExceptionDispatcher()
{
	__asm
	{
		add esp, 4

		mov eax, [esp]
		mov ebx, [esp + 4]
		push ebx
		push eax
		call custom_KiUserExceptionDispatcher_body

		test eax, eax
		jz continue_execution

		mov eax, [esp + 4]
		push 0
		push eax
		mov eax, ZwContinue_address
		call eax

		push eax
		call it_returned
		int 3

continue_execution:

		cld
		mov ecx, [esp + 4]
		mov ebx, [esp]
		push ecx
		push ebx

		mov eax, KiUserExceptionDispatcher_address
		add eax, 10
		jmp eax
	}
}

bool patch_exception_handler()
{
	/*
	unsigned address = reinterpret_cast<unsigned>(&custom_KiUserExceptionDispatcher);
	uchar * hack = reinterpret_cast<uchar *>(address);
	for(std::size_t i = 0; i < 4; i++)
	{
		uchar value = hack[i];
		if(value == '0x90' || value == '0xe9')
		{
			write_line("Invalid customised KiUserExceptionDispatcher address: " + ail::hex_string_32(address));
			return false;
		}
	}
	*/

	return patch_function("ntdll.dll", "KiUserExceptionDispatcher", KiUserExceptionDispatcher_address, &custom_KiUserExceptionDispatcher);
}

LONG WINAPI vectored_exception_handler(PEXCEPTION_POINTERS ExceptionInfo)
{
	//blah.
	DWORD const DBG_PRINTEXCEPTION_C = 0x40010006;

	EXCEPTION_RECORD & exception_record = *(ExceptionInfo->ExceptionRecord);
	CONTEXT & thread_context = *(ExceptionInfo->ContextRecord);

	//write_line("Exception type " + ail::hex_string_32(exception_record.ExceptionCode) + " at " + ail::hex_string_32(thread_context.Eip) + " in thread " + ail::hex_string_32(GetCurrentThreadId()));

	switch(exception_record.ExceptionCode)
	{
	case EXCEPTION_SINGLE_STEP:
		//error("DR!");
		perform_debug_register_check(thread_context);
		return EXCEPTION_CONTINUE_EXECUTION;

	case DBG_PRINTEXCEPTION_C:
		if(verbose)
			write_line("Debug message");
		break;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

bool install_exception_handler()
{
	if(!AddVectoredExceptionHandler(0, &vectored_exception_handler))
	{
		error("Failed to add vectored exception handler: " + ail::hex_string_32(GetLastError()));
		return false;
	}
	return true;
}
