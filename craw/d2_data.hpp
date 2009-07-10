#pragma once

#include <ail/types.hpp>

enum
{
	difficulty_count = 3
};

enum
{
	difficulty_normal,
	difficulty_nightmare,
	difficulty_hell
};

struct unit
{
	unsigned type;
	unsigned table_index;
	unsigned unknown0;
	unsigned id;
	unsigned mode;
	void * unit_data;
	unsigned act;
	void * act_data;
	unsigned seed[2];
	unsigned unknown1;
	void * path_data;
};

struct monster_data
{
	char unknown[22];
	uchar flags;

	bool is_minion();
	bool is_champion();
	bool is_boss();

	bool get_flag(unsigned offset);
};

struct path
{
	short offset_x;
	short position_x;
	short offset_y;
	short position_y;
};

struct treasure_class_entry
{
	ushort treasure_class[4];
};

struct monster_statistics
{
	char unused0[0x0c];

	unsigned flags;

	char unused1[0x86 - 0x10];

	treasure_class_entry treasure_classes[difficulty_count];

	char unused2[0xaa - 0x9e];

	ushort level[difficulty_count];
	ushort min_hp[difficulty_count];
	ushort max_hp[difficulty_count];

	char unused3[0xd4 - 0xbc];

	ushort experience[difficulty_count];

	char unused4[0x144 - 0xda];

	ushort damage_resistance[difficulty_count];
	ushort magic_resistance[difficulty_count];

	ushort fire_resistance[difficulty_count];
	ushort lightning_resistance[difficulty_count];
	ushort cold_resistance[difficulty_count];
	ushort poison_resistance[difficulty_count];

	char unused5[0x1a8 - 0x168];

	bool is_npc();
};

struct roster_unit
{
	char name[16];
	unsigned unit_id;
	unsigned life;
	unsigned unknown1;
	unsigned class_id;
	ushort level;
	ushort party_id;
	unsigned level_id;
	unsigned position_x;
	unsigned position_y;
	unsigned party_flags;
	uchar * unknown5;
	unsigned unknown6[11];
	ushort unknown7;
	char szName2[16];
	ushort unknown8;
	unsigned unknown9[2];
	roster_unit * next_roster;
};

monster_statistics & get_monster_statistics(std::size_t index);
roster_unit * get_player_roster(unsigned player_id);
unit * get_player();
