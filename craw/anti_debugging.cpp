#include <cstring>
#include <windows.h>
#include <ail/string.hpp>
#include "utility.hpp"

void is_debugger_present_hack()
{
	__asm
	{
		mov eax, fs:[018h]
		mov eax, [eax + 030h]
		mov byte ptr [eax + 2], 0
	}
}

void peb_nt_global_flags_hack()
{
	__asm
	{
		mov eax, fs:[030h] 
		and [eax + 068h], 0FFFFFF8Fh
	}
}

void heap_flags_hack()
{
	__asm
	{
		mov eax, fs:[030h] 
		mov eax, [eax + 018h]
		mov [eax + 010h], 0
	}
}

bool destroy_pe_header(void * module_base)
{
	DWORD
		old_protection,
		unused;
	if(!VirtualProtect(module_base, page_size, PAGE_READWRITE, &old_protection))
	{
		error("Failed to make PE header writable");
		return false;
	}
	std::memset(module_base, 0, page_size);
	if(!VirtualProtect(module_base, page_size, old_protection, &unused))
	{
		error("Failed to restore PE header protection");
		return false;
	}
	return true;
}

void anti_debugging()
{
	is_debugger_present_hack();
	peb_nt_global_flags_hack();
	heap_flags_hack();
}
