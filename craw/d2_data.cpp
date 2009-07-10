#include "d2_data.hpp"

bool read_flag(unsigned flags, unsigned offset)
{
	return ((flags >> offset) & 1) != 0;
}

bool monster_data::is_minion()
{
	return get_flag(4);
}

bool monster_data::is_champion()
{
	return get_flag(2);
}

bool monster_data::is_boss()
{
	return get_flag(3);
}

bool monster_data::get_flag(unsigned offset)
{
	return read_flag(flags, offset);
}

bool monster_statistics::is_npc()
{
	//use interact flag, haha.
	return read_flag(flags, 9);
}

monster_statistics & get_monster_statistics(std::size_t index)
{
	char * root = *reinterpret_cast<char **>(0x6FDEB500);
	monster_statistics * table = *reinterpret_cast<monster_statistics **>(root + 0xA78);
	return table[index];
}

roster_unit * get_player_roster(unsigned player_id)
{
	for(roster_unit * i = reinterpret_cast<roster_unit *>(0x6FBCC080); i; i = i->next_roster)
	{
		if(i->unit_id == player_id)
			return i;
	}
	return 0;
}

unit * get_player()
{
	return *reinterpret_cast<unit **>(0x6FBCC3D0);
}
