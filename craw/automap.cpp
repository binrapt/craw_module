/*
Command line arguments: python E:\Code\Python\ida\ida.py D2Client.dll B:\D2\D2Client.lst process_automap_unit automap_blobs E:\Code\craw_module\source\craw\automap.cpp
Time of generation: 2009-07-10 16:34:56
*/

#include <string>
#include <windows.h>
#include <ail/string.hpp>
#include "d2_data.hpp"
#include "utility.hpp"
#include "python.hpp"

namespace
{
	//Initialisation variables

	char const * module_name = "D2Client.dll";
	unsigned image_base = 0x6FAB0000;
	unsigned module_base = 0;

	unsigned get_y_coordinate = 0x6FABC214;
	unsigned sub_6FACF710 = 0x6FACF710;
	unsigned D2Common_10195 = 0x6FABC3D0;
	unsigned D2Win_10132 = 0x6FABD354;
	unsigned D2Lang_10005 = 0x6FABD1D4;
	unsigned draw_text = 0x6FABD360;
	unsigned sub_6FACF780 = 0x6FACF780;
	unsigned get_unit_state = 0x6FABC21A;
	unsigned D2Common_10860 = 0x6FABCA4E;
	unsigned automap_unit_type_check = 0x6FAEF3D0;
	unsigned sub_6FAEDE00 = 0x6FAEDE00;
	unsigned D2Common_10350 = 0x6FABC3D6;
	unsigned sub_6FAB2D40 = 0x6FAB2D40;
	unsigned draw_cross = 0x6FAED0C0;
	unsigned sub_6FACF3D0 = 0x6FACF3D0;
	unsigned set_text_size = 0x6FABD36C;
	unsigned get_x_coordinate = 0x6FABC208;

	//Custom variables

	unit * unit_pointer;
}

void automap_blobs_interrupt()
{
	__asm
	{
		int 3
	}
}

void __stdcall process_unit(int x, int y, uchar colour)
{
	python::perform_automap_callback(*unit_pointer, x, y, colour);
}

void __declspec(naked) intercept_draw_cross()
{
	__asm
	{
		pushad
		push [esp + 32 + 4]
		push eax
		push ecx
		call process_unit
		popad
		ret 4
	}
}

void automap_blobs();

void __stdcall initialise_automap_blobs()
{
	module_base = reinterpret_cast<unsigned>(GetModuleHandle(module_name));
	if(module_base == 0)
		automap_blobs_interrupt();
	
	unsigned * call_addresses[] =
	{
		&get_y_coordinate,
		&sub_6FACF710,
		&D2Common_10195,
		&D2Win_10132,
		&D2Lang_10005,
		&draw_text,
		&sub_6FACF780,
		&get_unit_state,
		&D2Common_10860,
		&automap_unit_type_check,
		&sub_6FAEDE00,
		&D2Common_10350,
		&sub_6FAB2D40,
		&draw_cross,
		&sub_6FACF3D0,
		&set_text_size,
		&get_x_coordinate
	};
	
	unsigned linking_offset = module_base - image_base;
	
	for(std::size_t i = 0; i < 17; i++)
	{
		unsigned & address = *call_addresses[i];
		address += linking_offset;
	}
	
	bool success = false;
	
	std::string const marker = "\x0f\x0b\x0f\x0b\x0f\x0b\x0f\x0b";
	
	char * data_pointer = reinterpret_cast<char *>(&automap_blobs);
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
		automap_blobs_interrupt();
	
	data_pointer += marker.size();
	
	for(unsigned i = 0; i < 19; i++)
	{
		char * label_pointer = *reinterpret_cast<char **>(data_pointer + 1);
		unsigned * immediate_pointer = reinterpret_cast<unsigned *>(label_pointer - 4);
		DWORD old_protection;
		SIZE_T const patch_size = 4;
		if(!VirtualProtect(immediate_pointer, patch_size, PAGE_EXECUTE_READWRITE, &old_protection))
			automap_blobs_interrupt();
		unsigned & address = *immediate_pointer;
		address += linking_offset;
		DWORD unused;
		if(!VirtualProtect(immediate_pointer, patch_size, old_protection, &unused))
			automap_blobs_interrupt();
		data_pointer += 5;
	}
}

void __declspec(naked) automap_blobs()
{
	__asm
	{
		//Initialisation code:
		
		cmp module_base, 0
		jnz is_already_initialised
		
		call initialise_automap_blobs
		
	is_already_initialised:

		//store the unit pointer so we can restore it later if necessary
		mov unit_pointer, esi
	
		//Actual code starts here:
		
		sub esp, 10h
		push ebp
		push esi
		mov esi, eax
		test esi, esi
		mov ebp, 6
		jz loc_6FAEF932
		mov ebp, ds:[esi]
	loc_6FAEF932:
		test ebp, ebp
		mov byte ptr ds:[esp + 18h - 10h], 0
		jnz loc_6FAEF975
		test dword ptr ds:[esi + 0C4h], 10000h
		jnz loc_6FAEF965
		mov eax, ds:[esi]
		sub eax, ebp
		mov ecx, ds:[esi + 10h]
		jz loc_6FAEF95C
		dec eax
		jnz loc_6FAEF975
		test ecx, ecx
		jz loc_6FAEF965
		cmp ecx, 0Ch
		jmp loc_6FAEF963
	loc_6FAEF95C:
		test ecx, ecx
		jz loc_6FAEF965
		cmp ecx, 11h
	loc_6FAEF963:
		jnz loc_6FAEF975
	loc_6FAEF965:
		push 7
		push esi
		call get_unit_state
		test eax, eax
		jz loc_6FAEFC2C
	loc_6FAEF975:
		push ebx
		lea ebx, ds:[esp + 1Ch - 10h]

		/*
		mov eax, esi
		call automap_unit_type_check
		*/

		//<modification>

		mov eax, esi
		call automap_unit_type_check

		//fix esi and always have the check pass, keep the colour around though
		mov esi, unit_pointer
		mov eax, 1

		//</modification>

		test eax, eax
		jz dont_draw_unit
		push edi
		push esi
		call get_x_coordinate
		push esi
		mov edi, eax
		call get_y_coordinate
		mov ebx, ds:[06FBA3B98h]
	linker_address_0:
		mov ecx, eax
		mov eax, edi
		cdq
		idiv ebx
		mov edx, ds:[06FBCC2E8h]
	linker_address_1:
		mov edi, eax
		sub edi, edx
		mov eax, ecx
		cdq
		idiv ebx
		mov edx, ds:[06FBCC2ECh]
	linker_address_2:
		add edi, 8
		mov ebx, eax
		mov eax, ds:[06FBCC318h]
	linker_address_3:
		sub ebx, edx
		sub ebx, 8
		cmp edi, eax
		jl loc_6FAEFC2A
		cmp edi, ds:[06FBCC320h]
	linker_address_4:
		jg loc_6FAEFC2A
		cmp ebx, ds:[06FBCC31Ch]
	linker_address_5:
		jl loc_6FAEFC2A
		cmp ebx, ds:[06FBCC324h]
	linker_address_6:
		jg loc_6FAEFC2A
		mov eax, ebp
		sub eax, 0
		jz loc_6FAEFBCD
		dec eax
		jz loc_6FAEFA80
		dec eax
		jnz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA4h]
	linker_address_7:
		test eax, eax
		jz loc_6FAEFA6A
		test esi, esi
		jz loc_6FAEFA6A
		cmp dword ptr ds:[esi + 4], 10Bh
		jnz loc_6FAEFA6A
		mov ecx, 6
		call set_text_size
		mov ecx, 0CF3h
		mov ebp, eax
		call D2Lang_10005
		mov esi, eax
		movzx eax, byte ptr ds:[esp + 20h - 10h]
		push 0
		push eax
		add ebx, 0FFFFFFF6h
		push ebx
		mov ecx, esi
		call D2Win_10132
		cdq
		sub eax, edx
		sar eax, 1
		mov edx, edi
		sub edx, eax
		mov ecx, esi
		call draw_text
		mov ecx, ebp
		call set_text_size
		pop edi
		pop ebx
		pop esi
		pop ebp
		add esp, 10h
		retn
	loc_6FAEFA6A:
		mov ecx, ds:[esp + 20h - 10h]
		push ecx
		mov eax, ebx
		mov ecx, edi

		//call draw_cross
		call intercept_draw_cross

		pop edi
		pop ebx
		pop esi
		pop ebp
		add esp, 10h
		retn
	loc_6FAEFA80:
		mov edx, ds:[esp + 20h - 10h]
		push edx
		mov eax, ebx
		mov ecx, edi

		//call draw_cross
		call intercept_draw_cross

		mov eax, ds:[06FBA3BA4h]
	linker_address_8:
		test eax, eax
		jz loc_6FAEFC2A
		test esi, esi
		jnz loc_6FAEFAA4
		or eax, 0FFFFFFFFh
		jmp loc_6FAEFAA7
	loc_6FAEFAA4:
		mov eax, ds:[esi + 4]
	loc_6FAEFAA7:
		mov edx, 9
		call sub_6FAB2D40
		test eax, eax
		jz loc_6FAEFAF4
		mov ecx, 6
		call set_text_size
		mov ds:[esp + 20h - 10h], eax
		mov eax, esi
		call sub_6FACF3D0
		push 0
		mov ebp, eax
		push 4
		lea eax, ds:[ebx - 0Ah]
		push eax
		mov ecx, ebp
		call D2Win_10132
		cdq
		sub eax, edx
		sar eax, 1
		mov edx, edi
		sub edx, eax
		mov ecx, ebp
		call draw_text
		mov ecx, ds:[esp + 20h - 10h]
		call set_text_size
	loc_6FAEFAF4:
		mov eax, ds:[06FBA3BA4h]
	linker_address_9:
		test eax, eax
		jz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA0h]
	linker_address_10:
		test eax, eax
		jz loc_6FAEFC2A
		test esi, esi
		mov eax, 6
		jz loc_6FAEFB19
		mov eax, ds:[esi]
	loc_6FAEFB19:
		test esi, esi
		mov ds:[esp + 20h - 10h], eax
		jnz loc_6FAEFB26
		or eax, 0FFFFFFFFh
		jmp loc_6FAEFB29
	loc_6FAEFB26:
		mov eax, ds:[esi + 4]
	loc_6FAEFB29:
		test esi, esi
		mov ds:[esp + 20h - 8], eax
		jnz loc_6FAEFB35
		xor eax, eax
		jmp loc_6FAEFB38
	loc_6FAEFB35:
		mov eax, ds:[esi + 10h]
	loc_6FAEFB38:
		push 0A15h
		push 06FB85D18h
	linker_address_11:
		lea ecx, ds:[esp + 28h - 0Ch]
		push ecx
		lea edx, ds:[esp + 2Ch - 8]
		mov ds:[esp + 2Ch - 0Ch], eax
		push edx
		lea eax, ds:[esp + 30h - 10h]
		push eax
		push esi
		call D2Common_10350
		mov eax, ds:[esp + 20h - 10h]
		test eax, eax
		jnz loc_6FAEFC2A
		push 25h
		push esi
		call D2Common_10195
		test eax, eax
		jz loc_6FAEFC2A
		mov eax, esi
		call sub_6FACF710
		mov ebp, eax
		test ebp, ebp
		jz loc_6FAEFC2A
		cmp dword ptr ds:[ebp + 0], 0
		jnz loc_6FAEFC2A
		mov eax, ds:[06FBCC3D0h]
	linker_address_12:
		call sub_6FACF780
		cmp ax, 0FFFFh
		mov ds:[esp + 20h - 4], eax
		jz loc_6FAEFBB4
		mov eax, esi
		call sub_6FACF780
		cmp word ptr ds:[esp + 20h - 4], ax
		jz loc_6FAEFC2A
	loc_6FAEFBB4:
		push 1
		push ebx
		push edi
		push ebp
		call D2Common_10860
		mov ebx, eax
		call sub_6FAEDE00
		pop edi
		pop ebx
		pop esi
		pop ebp
		add esp, 10h
		retn
	loc_6FAEFBCD:
		mov eax, ds:[06FBA3BA0h]
	linker_address_13:
		test eax, eax
		jnz loc_6FAEFBE2
		mov cl, byte ptr ds:[esp + 20h - 10h]
		cmp cl, ds:[06FBCC303h]
	linker_address_14:
		jz loc_6FAEFBF0
	loc_6FAEFBE2:
		mov edx, ds:[esp + 20h - 10h]
		push edx
		mov eax, ebx
		mov ecx, edi
		
		//call draw_cross
		call intercept_draw_cross

	loc_6FAEFBF0:
		mov eax, ds:[06FBA3BA4h]
	linker_address_15:
		test eax, eax
		jz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA0h]
	linker_address_16:
		test eax, eax
		jz loc_6FAEFC2A
		cmp esi, ds:[06FBCC3D0h]
	linker_address_17:
		jz loc_6FAEFC2A
		mov al, ds:[06FBCC303h]
	linker_address_18:
		cmp byte ptr ds:[esp + 20h - 10h], al
		jnz loc_6FAEFC19
		push 2
		jmp loc_6FAEFC1B
	loc_6FAEFC19:
		push 1
	loc_6FAEFC1B:
		push ebx
		push edi
		push esi
		call D2Common_10860
		mov ebx, eax
		call sub_6FAEDE00
	loc_6FAEFC2A:
		pop edi
	dont_draw_unit:
		pop ebx
	loc_6FAEFC2C:
		pop esi
		pop ebp
		add esp, 10h
		retn

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
		push linker_address_6
		push linker_address_7
		push linker_address_8
		push linker_address_9
		push linker_address_10
		push linker_address_11
		push linker_address_12
		push linker_address_13
		push linker_address_14
		push linker_address_15
		push linker_address_16
		push linker_address_17
		push linker_address_18
	}
}

