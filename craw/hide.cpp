#include <windows.h>
#include <cstring>
#include "hide.hpp"

//from Darawk

struct UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
};

struct MODULE_INFO_NODE
{
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
	HMODULE baseAddress;
	unsigned long entryPoint;
	unsigned int size;
	UNICODE_STRING fullPath;
	UNICODE_STRING name;
	unsigned long flags;
	unsigned short LoadCount;
	unsigned short TlsIndex;
	LIST_ENTRY HashTable;
	unsigned long timestamp;
};

struct PROCESS_MODULE_INFO
{
	unsigned int size;
	unsigned int initialized;
	HANDLE SsHandle;
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
};

void unlink_entry(LIST_ENTRY & entry)
{
	entry.Blink->Flink = entry.Flink;
	entry.Flink->Blink = entry.Blink;
}

bool hide_module(void * module_base)
{
	PROCESS_MODULE_INFO * process_module_pointer;
	MODULE_INFO_NODE * module;

	_asm
	{
		mov eax, fs:[18h]
		mov eax, [eax + 30h]
		mov eax, [eax + 0Ch]
		mov process_module_pointer, eax
	}

    module = (MODULE_INFO_NODE *)(process_module_pointer->LoadOrder.Flink);
	
	while(true)
	{
		HMODULE base_address = module->baseAddress;
		if(base_address == 0)
			return false;
		if(module->baseAddress == module_base)
			break;
		module = (MODULE_INFO_NODE *)(module->LoadOrder.Flink);
	}

	if(!module->baseAddress)
		return false;

	unlink_entry(module->LoadOrder);
	unlink_entry(module->InitOrder);
	unlink_entry(module->MemoryOrder);
	unlink_entry(module->HashTable);

	std::memset(module->fullPath.Buffer, 0, module->fullPath.Length);
	std::memset(module, 0, sizeof(MODULE_INFO_NODE));

	return true;
}
