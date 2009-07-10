/*
Command line arguments: python E:\Code\Python\ida\ida.py D2Net.dll B:\D2\D2Net.lst 6FBF6650 6FBF67DC main_packet_handler E:\Code\craw_module\source\packet_handler.cpp
Time of generation: 2009-07-07 20:45:47
*/

#include <string>
#include <windows.h>

namespace
{
	//Initialisation variables

	char const * module_name = "D2Net.dll";
	unsigned image_base = 0x6FBF0000;
	unsigned module_base = 0;

	unsigned Fog_10265 = 0x6FBF5EA0;
	unsigned Fog_10024 = 0x6FBF5E64;
	unsigned Ordinal10002 = 0x6FBF6090;
	unsigned Fog_10219 = 0x6FBF5F00;
	unsigned _memcpy = 0x6FBF11C0;
	unsigned Fog_10042 = 0x6FBF5EE8;
	unsigned custom_exit = 0x6FBF1187;

}

void interrupt()
{
	__asm
	{
		int 3
	}
}

void main_packet_handler();

void __stdcall initialisation()
{
	module_base = reinterpret_cast<unsigned>(GetModuleHandle(module_name));
	if(module_base == 0)
		interrupt();
	
	unsigned * call_addresses[] =
	{
		&Fog_10265,
		&Fog_10024,
		&Ordinal10002,
		&Fog_10219,
		&_memcpy,
		&Fog_10042,
		&custom_exit
	};
	
	unsigned linking_offset = module_base - image_base;
	
	for(std::size_t i = 0; i < 7; i++)
	{
		unsigned & address = *call_addresses[i];
		address += linking_offset;
	}
	
	bool success = false;
	
	std::string const marker = "\x0f\x0b\x0f\x0b\x0f\x0b\x0f\x0b";
	
	char * data_pointer = reinterpret_cast<char *>(&main_packet_handler);
	while(true)
	{
		std::string current_string(data_pointer, marker.size());
		if(current_string == marker)
		{
			success = true;
			break;
		}
		data_pointer++;
	}
	
	if(!success)
		interrupt();
	
	data_pointer += marker.size();
	
	for(unsigned i = 0; i < 6; i++)
	{
		char * label_pointer = *reinterpret_cast<char **>(data_pointer + 1);
		unsigned * immediate_pointer = reinterpret_cast<unsigned *>(label_pointer - 4);
		DWORD old_protection;
		SIZE_T const patch_size = 4;
		if(!VirtualProtect(immediate_pointer, patch_size, PAGE_EXECUTE_READWRITE, &old_protection))
			interrupt();
		unsigned & address = *immediate_pointer;
		address += linking_offset;
		DWORD unused;
		if(!VirtualProtect(immediate_pointer, patch_size, old_protection, &unused))
			interrupt();
		data_pointer += 5;
	}
}
void __declspec(naked) main_packet_handler()
{
	__asm
	{
		//Initialisation code:
		
		cmp module_base, 0
		jnz is_already_initialised
		
		call initialisation
		
	is_already_initialised:
	
		//Actual code starts here:
		
		sub esp, 110h
		mov eax, ds:[06FBFB244h]
	linker_address_0:
		push ebx
		push ebp
		push esi
		push edi
		mov edi, ds:[eax + 7B8h]
		cmp edi, 1
		mov esi, eax
		mov ds:[esp + 14h], edi
		mov ds:[esp + 10h], esi
		mov dword ptr ds:[esp + 18h], 0
		jb loc_6FBF6795
	loc_6FBF6680:
		lea eax, ds:[esp + 1Ch]
		push eax
		mov edx, edi
		mov ecx, esi
		call Ordinal10002
		test eax, eax
		jz loc_6FBF6790
		mov ebx, ds:[esp + 1Ch]
		cmp ebx, 204h
		ja loc_6FBF67BD
		cmp ebx, edi
		jg loc_6FBF6790
		push 0
		push 12Ah
		mov edx, 06FBF9264h
	linker_address_1:
		mov ecx, 210h
		call Fog_10042
		mov ebp, eax
		xor eax, eax
		mov ecx, ebx
		mov edx, ecx
		shr ecx, 2
		mov ds:[ebp + 204h], ebx
		mov ds:[ebp + 208h], eax
		mov ds:[ebp + 20Ch], eax
		mov edi, ebp
		rep movsd
		mov ecx, edx
		and ecx, 3
		rep movsb
		mov edi, ds:[esp + 10h]
		mov al, ds:[edi]
		cmp al, 0AFh
		jnz loc_6FBF673F
		mov cl, ds:[edi + 1]
		test cl, cl
		lea esi, ds:[edi + 1]
		jz loc_6FBF673F
		cmp cl, 81h
		mov dword ptr ds:[esp + 18h], 1
		jnz loc_6FBF674C
		lea ecx, ds:[esp + 21h]
		mov edi, 80h
	loc_6FBF6716:
		mov al, ds:[esi]
		mov dl, al
		and dl, 0Fh
		inc dl
		shr al, 4
		inc al
		mov ds:[ecx - 1], dl
		mov ds:[ecx], al
		inc esi
		add ecx, 2
		dec edi
		jnz loc_6FBF6716
		lea ecx, ds:[esp + 20h]
		call Fog_10219
		mov edi, ds:[esp + 10h]
		jmp loc_6FBF674C
	loc_6FBF673F:
		cmp al, 8Fh
		jnz loc_6FBF674C
		call ds:GetTickCount
		mov ds:[ebp + 0Dh], eax
	loc_6FBF674C:
		cmp byte ptr ds:[edi], 0AFh
		mov eax, 06FBFB3FCh
	linker_address_2:
		jb loc_6FBF675B
		mov eax, 06FBFB3F4h
	linker_address_3:
	loc_6FBF675B:
		cmp dword ptr ds:[eax], 0
		jz loc_6FBF6771
	loc_6FBF6760:
		mov eax, ds:[eax]
		mov ecx, ds:[eax + 20Ch]
		add eax, 20Ch
		test ecx, ecx
		jnz loc_6FBF6760
	loc_6FBF6771:
		mov ds:[eax], ebp
		mov eax, ds:[esp + 14h]
		add edi, ebx
		sub eax, ebx
		cmp eax, 1
		mov ds:[esp + 10h], edi
		mov esi, edi
		mov ds:[esp + 14h], eax
		mov edi, eax
		jnb loc_6FBF6680
	loc_6FBF6790:
		mov eax, ds:[06FBFB244h]
	linker_address_4:
	loc_6FBF6795:
		test edi, edi
		mov ds:[eax + 7B8h], edi
		jz loc_6FBF67AE
		cmp esi, eax
		jz loc_6FBF67AE
		push edi
		push esi
		push eax
		call _memcpy
		add esp, 0Ch
	loc_6FBF67AE:
		mov eax, ds:[esp + 18h]
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 110h
		retn
	loc_6FBF67BD:
		push 127h
		call Fog_10265
		push eax
		push 06FBF9203h
	linker_address_5:
		call Fog_10024
		add esp, 0Ch
		push 0FFFFFFFFh
		call custom_exit

		//Instruction address table hack:

		ud2
		ud2
		ud2
		ud2

		push linker_address_0
		push linker_address_1
		push linker_address_2
		push linker_address_3
		push linker_address_4
		push linker_address_5
	}
}

