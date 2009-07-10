/*
Command line arguments: python E:\Code\Python\ida\ida.py B:\D2\D2Client.lst process_automap_unit automap_blobs E:\Code\craw_module\source\automap.cpp
Time of generation: 2009-07-06 20:56:11
*/

#include <windows.h>
#include <ail/types.hpp>
#include <ail/array.hpp>
#include <ail/string.hpp>
#include "d2_data.hpp"
#include "d2_functions.hpp"
#include "python.hpp"

namespace
{
	unsigned get_y_coordinate = 0x6FABC214;
	unsigned sub_6FACF710 = 0x6FACF710;
	unsigned D2Common_10195 = 0x6FABC3D0;
	unsigned D2Win_10132 = 0x6FABD354;
	unsigned D2Lang_10005 = 0x6FABD1D4;
	unsigned draw_text_import = 0x6FABD360;
	unsigned sub_6FACF780 = 0x6FACF780;
	unsigned get_unit_state = 0x6FABC21A;
	unsigned D2Common_10860 = 0x6FABCA4E;
	//unsigned automap_unit_type_check = 0x6FAEF3D0;
	unsigned original_automap_unit_type_check = 0x6FAEF3D0;
	unsigned sub_6FAEDE00 = 0x6FAEDE00;
	unsigned D2Common_10350 = 0x6FABC3D6;
	unsigned is_npc = 0x6FAB2D40;
	unsigned draw_cross = 0x6FAED0C0;
	//unsigned sub_6FACF3D0 = 0x6FACF3D0;
	unsigned get_unicode_name_string = 0x6FACF3D0;
	unsigned set_text_size = 0x6FABD36C;
	unsigned get_x_coordinate = 0x6FABC208;

	unsigned
		npc_colour = 0x20,

		self_colour = 0x97,
		allied_player_colour = 0x84,
		unpartied_player_colour = 0xe5,
		hostile_player_colour = 0x62,

		monster_colour = 0x0a,
		minion_colour = 0xab,
		champion_colour = 0x9b,
		boss_colour = 0x87;

	unsigned draw_symbol = draw_cross;

	bool draw_custom_string;
	std::string custom_string;
}

void __stdcall draw_box_and_text(unit * unit_pointer, int x, int y, unsigned colour)
{
	unit & current_unit = *unit_pointer;
	monster_statistics & statistics = get_monster_statistics(current_unit.table_index);
	python::perform_automap_callback(current_unit, statistics, x, y);

	/*
	draw_box(x, y, colour);
	if(draw_custom_string)
		draw_text(custom_string, x - 5, y - 2, 0, true);
	*/
}

void __declspec(naked) draw_box_stub()
{
	__asm
	{
		pushad

		mov ebx, [esp + 32 + 4]
		
		push ebx
		push eax
		push ecx
		push esi
		call draw_box_and_text
		
		popad
		ret 4
	}
}

unsigned __stdcall process_unit(unit * unit_pointer, unsigned * colour_pointer)
{
	unit & current_unit = *unit_pointer;
	unsigned & colour = *colour_pointer;

	draw_custom_string = false;
	draw_symbol = reinterpret_cast<unsigned>(&draw_box_stub);

	//?
	colour = 0;

	if(current_unit.mode == 0 || current_unit.mode == 12)
		//dead
		return 0;

	monster_statistics & statistics = get_monster_statistics(current_unit.table_index);
	if(statistics.is_npc())
	{
		colour = npc_colour;
		return 1;
	}

	if(statistics.treasure_classes[0].treasure_class[0] == 0)
		return 0;

	monster_data & data = *reinterpret_cast<monster_data *>(current_unit.unit_data);

	if(data.is_boss())
		colour = boss_colour;
	else if(data.is_champion())
		colour = champion_colour;
	else if(data.is_minion())
		colour = minion_colour;
	else
		colour = monster_colour;

	std::string description;

	unsigned immunity_colours[] =
	{
		4,
		3,
		1,
		9,
		2,
		8
	};

	unsigned immunity_stats[] =
	{
		36,
		43,
		39,
		41,
		45,
		37
	};

	for(std::size_t i = 0; i < ail::countof(immunity_colours); i++)
	{
		unsigned stat = d2_get_unit_stat(unit_pointer, immunity_stats[i], 0);
		if(stat >= 100)
		{
			std::string colour_string = "\xff" "c" + ail::number_to_string(immunity_colours[i]) + "o";
			description += colour_string;
		}
	}

	custom_string = description;
	draw_custom_string = !custom_string.empty();

	return 1;
}		

void __declspec(naked) automap_unit_type_check()
{
	/*
	__asm
	{
		mov eax, 1
		ret
	}
	*/

	__asm
	{
		cmp dword ptr [esi], 1
		jnz process_player

		pushad
		push ebx
		push esi
		call process_unit
		mov [esp + 28], eax
		popad
		ret

	process_player:
		jmp original_automap_unit_type_check
	}
}

void __declspec(naked) automap_blobs()
{
	__asm
	{
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
		mov eax, esi
		call automap_unit_type_check
		test eax, eax
		jz dont_draw_unit
		push edi
		push esi
		call get_x_coordinate
		push esi
		mov edi, eax
		call get_y_coordinate
		mov ebx, ds:[06FBA3B98h]
		mov ecx, eax
		mov eax, edi
		cdq
		idiv ebx
		mov edx, ds:[06FBCC2E8h]
		mov edi, eax
		sub edi, edx
		mov eax, ecx
		cdq
		idiv ebx
		mov edx, ds:[06FBCC2ECh]
		add edi, 8
		mov ebx, eax
		mov eax, ds:[06FBCC318h]
		sub ebx, edx
		sub ebx, 8
		cmp edi, eax
		jl loc_6FAEFC2A
		cmp edi, ds:[06FBCC320h]
		jg loc_6FAEFC2A
		cmp ebx, ds:[06FBCC31Ch]
		jl loc_6FAEFC2A
		cmp ebx, ds:[06FBCC324h]
		jg loc_6FAEFC2A
		mov eax, ebp
		sub eax, 0
		jz loc_6FAEFBCD
		dec eax
		jz loc_6FAEFA80
		dec eax
		jnz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA4h]
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
		call draw_text_import
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
		call draw_symbol

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
		call draw_symbol

		mov eax, ds:[06FBA3BA4h]
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
		call is_npc
		test eax, eax
		jz loc_6FAEFAF4
		mov ecx, 6
		call set_text_size
		mov ds:[esp + 20h - 10h], eax
		mov eax, esi
		call get_unicode_name_string
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
		call draw_text_import
		mov ecx, ds:[esp + 20h - 10h]
		call set_text_size
	loc_6FAEFAF4:
		mov eax, ds:[06FBA3BA4h]
		test eax, eax
		jz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA0h]
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
		test eax, eax
		jnz loc_6FAEFBE2
		mov cl, byte ptr ds:[esp + 20h - 10h]
		cmp cl, ds:[06FBCC303h]
		jz loc_6FAEFBF0
	loc_6FAEFBE2:
		mov edx, ds:[esp + 20h - 10h]
		push edx
		mov eax, ebx
		mov ecx, edi

		//call draw_cross
		call draw_symbol

	loc_6FAEFBF0:
		mov eax, ds:[06FBA3BA4h]
		test eax, eax
		jz loc_6FAEFC2A
		mov eax, ds:[06FBA3BA0h]
		test eax, eax
		jz loc_6FAEFC2A
		cmp esi, ds:[06FBCC3D0h]
		jz loc_6FAEFC2A
		mov al, ds:[06FBCC303h]
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
	}
}
