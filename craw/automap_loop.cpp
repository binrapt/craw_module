/*
Command line arguments: python E:\Code\Python\ida\ida.py B:\D2\D2Client.lst sub_6FAF0350 automap_loop E:\Code\craw_module\source\automap_loop.cpp
Time of generation: 2009-07-04 18:15:11
*/

#include <cstddef>
#include "automap.hpp"

namespace
{
	//unsigned sub_6FAEF920 = 0x6FAEF920;
	unsigned sub_6FAEF920 = reinterpret_cast<unsigned>(&automap_blobs);
	unsigned _exit = 0x6FAB326D;
	unsigned Fog_10265 = 0x6FABBF8C;
	unsigned D2Common_10366 = 0x6FABC1BA;
	unsigned Fog_10024 = 0x6FABBF80;
	unsigned D2Common_10915 = 0x6FABC6A0;
	unsigned sub_6FAEE860 = 0x6FAEE860;

}
void __declspec(naked) automap_loop()
{
	__asm
	{
		sub esp, 8
		mov eax, ds:[06FBCC3D0h]
		push ebx
		push ebp
		mov ebp, ds:[06FBCC080h]
		push esi
		push edi
		lea ecx, ds:[esp + 18h - 8]
		push ecx
		lea edx, ds:[esp + 1Ch - 4]
		push edx
		xor edi, edi
		push eax
		mov ds:[esp + 24h - 4], edi
		mov ds:[esp + 24h - 8], edi
		call D2Common_10366
		push eax
		call D2Common_10915
		mov eax, ds:[esp + 18h - 8]
		cmp eax, edi
		mov ebx, eax
		jle loc_6FAF03B6
		lea esp, ds:[esp + 0]
	loc_6FAF0390:
		mov eax, ds:[esp + 18h - 4]
		mov ecx, ds:[eax + edi * 4]
		mov esi, ds:[ecx + 40h]
		test esi, esi
		jz loc_6FAF03B1
		mov edi, edi
	loc_6FAF03A0:
		mov eax, esi
		call sub_6FAEF920
		mov esi, ds:[esi + 0E8h]
		test esi, esi
		jnz loc_6FAF03A0
	loc_6FAF03B1:
		inc edi
		cmp edi, ebx
		jl loc_6FAF0390
	loc_6FAF03B6:
		mov esi, ds:[06FBCC080h]
		xor edx, edx
		test ebp, ebp
		mov eax, esi
		jz loc_6FAF04AE
		test esi, esi
		jz loc_6FAF04AE
		mov cx, ds:[ebp + 22h]
	loc_6FAF03D4:
		cmp ds:[eax + 22h], cx
		jnz loc_6FAF03DB
		inc edx
	loc_6FAF03DB:
		mov eax, ds:[eax + 80h]
		test eax, eax
		jnz loc_6FAF03D4
		cmp edx, 1
		jle loc_6FAF04AE
		mov ebx, ecx
		cmp bx, 0FFFFh
		jz loc_6FAF04AE
		mov eax, ds:[06FBA3BA0h]
		test eax, eax
		jz loc_6FAF04AE
		mov edi, ds:[ebp + 80h]
		test edi, edi
		jz loc_6FAF04AE
	loc_6FAF0416:
		mov ecx, ds:[edi + 10h]
		mov edx, ecx
		and edx, 7Fh
		mov eax, ds:[edx * 4 + 06FBCA960h]
		test eax, eax
		jz loc_6FAF043F
		lea esp, ds:[esp + 0]
	loc_6FAF0430:
		cmp ds:[eax + 0Ch], ecx
		jz loc_6FAF046D
		mov eax, ds:[eax + 0E4h]
		test eax, eax
		jnz loc_6FAF0430
	loc_6FAF043F:
		xor edx, edx
		test esi, esi
		mov eax, esi
		jz loc_6FAF0466
		mov cx, ds:[edi + 22h]
		jmp loc_6FAF0450
		align 10h
	loc_6FAF0450:
		cmp ds:[eax + 22h], cx
		jnz loc_6FAF0457
		inc edx
	loc_6FAF0457:
		mov eax, ds:[eax + 80h]
		test eax, eax
		jnz loc_6FAF0450
		cmp edx, 1
		jg loc_6FAF048E
	loc_6FAF0466:
		mov eax, 0FFFFh
		jmp loc_6FAF0490
	loc_6FAF046D:
		cmp dword ptr ds:[eax], 0
		jz loc_6FAF04A0
		push 5Ah
		call Fog_10265
		push eax
		push 06FB80323h
		call Fog_10024
		add esp, 0Ch
		push 0FFFFFFFFh
		call _exit
	loc_6FAF048E:
		mov eax, ecx
	loc_6FAF0490:
		cmp bx, ax
		jnz loc_6FAF04A0
		call sub_6FAEE860
		mov esi, ds:[06FBCC080h]
	loc_6FAF04A0:
		mov edi, ds:[edi + 80h]
		test edi, edi
		jnz loc_6FAF0416
	loc_6FAF04AE:
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 8
		retn
	}
}
