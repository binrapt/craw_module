#include <windows.h>
#include <ail/array.hpp>
#include <ail/string.hpp>
#include "d2_functions.hpp"
#include "utility.hpp"

class module_offset_handler
{
public:
	module_offset_handler(unsigned image_base, unsigned module_base):
		image_base(image_base),
		module_base(module_base)
	{
	}

	template <typename type>
		void fix(type & function, unsigned offset)
	{
		function = reinterpret_cast<type>(module_base + offset - image_base);
	}

private:
	unsigned
		image_base,
		module_base;
};

namespace
{
	unsigned
		d2win_base = 0x6F8E0000,
		d2gfx_base = 0x6FA80000,
		d2common_base = 0x6FD50000,
		d2client_base = 0x6FAB0000;

	unsigned const life_mana_shift = 8;
}

//D2Win.dll
set_font_size_type d2_set_font_size;
draw_text_type d2_draw_text;
get_text_width_type d2_get_text_width;

//D2Gfx.dll
draw_line_type d2_draw_line;

//D2Common.dll
get_unit_stat_type d2_get_unit_stat;

//D2Client.dll
get_player_unit_type d2_get_player_unit;
get_difficulty_type d2_get_difficulty;

void initialise_d2win_functions(unsigned base)
{
	module_offset_handler offset_handler(d2win_base, base);

	offset_handler.fix(d2_set_font_size, 0x6F8F08D0);
	offset_handler.fix(d2_draw_text, 0x6F8F0890);
	offset_handler.fix(d2_get_text_width, 0x6F8EFFF0);
}

void initialise_d2gfx_functions(unsigned base)
{
	module_offset_handler offset_handler(d2gfx_base, base);

	offset_handler.fix(d2_draw_line, 0x6FA87CA0);
}

void initialise_d2common_functions(unsigned base)
{
	module_offset_handler offset_handler(d2common_base, base);

	offset_handler.fix(d2_get_unit_stat, 0x6FD84E20);
}

void initialise_d2client_functions(unsigned base)
{
	module_offset_handler offset_handler(d2client_base, base);

	offset_handler.fix(d2_get_player_unit, 0x6FACE490);
	offset_handler.fix(d2_get_difficulty, 0x6FACE490);
}

void draw_text(std::string const & text, int x, int y, unsigned colour, bool centered)
{
	std::size_t buffer_size = text.size() + 1;
	wchar_t * wide_string_data = new wchar_t[buffer_size];

	std::memset(wide_string_data, 0, buffer_size * 2);
	MultiByteToWideChar(0, MB_PRECOMPOSED, text.c_str(), text.length(), wide_string_data, buffer_size);

	unsigned font_size = 6;

	unsigned old_font_size = d2_set_font_size(font_size);
	if(centered)
	{
		unsigned
			width,
			file_number;

		d2_get_text_width(wide_string_data, &width, &file_number);

		x -= width / 2;
	}

	d2_draw_text(wide_string_data, x, y, colour, -1);
	d2_set_font_size(old_font_size);

	delete wide_string_data;
}

struct coordinate
{
	int
		x,
		y;

	coordinate(int x, int y):
		x(x),
		y(y)
	{
	}
};

void __stdcall draw_box(int x, int y, unsigned colour)
{
	bool const continuous = true;

	int const square_size = 3;

	coordinate const coordinates[] =
	{
		coordinate(- square_size, square_size),
		coordinate(square_size, square_size),
		coordinate(square_size, - square_size),
		coordinate(- square_size, - square_size),
		coordinate(- square_size, square_size)
	};

	std::size_t
		limit,
		increment;

	if(continuous)
	{
		limit = ail::countof(coordinates) - 1;
		increment = 1;
	}
	else
	{
		limit = ail::countof(coordinates);
		increment = 2;
	}

	for(std::size_t i = 0; i < limit; i += increment)
	{
		coordinate const & start = coordinates[i];
		coordinate const & end = coordinates[i + 1];
		d2_draw_line(x + start.x, y + start.y, x + end.x, y + end.y, colour, 0xff);
	}
}

bool get_life(unsigned & current_life, unsigned & maximum_life)
{
	unit * player_unit = d2_get_player_unit();
	if(player_unit == 0)
		return false;
	current_life = d2_get_unit_stat(player_unit, 6, 0) >> life_mana_shift;
	maximum_life = d2_get_unit_stat(player_unit, 7, 0) >> life_mana_shift;
	return true;
}
