#pragma once

#include <string>
#include <ail/types.hpp>
#include "d2_data.hpp"

void initialise_d2win_functions(unsigned base);
void initialise_d2gfx_functions(unsigned base);
void initialise_d2common_functions(unsigned base);
void initialise_d2client_functions(unsigned base);

typedef unsigned (__fastcall * set_font_size_type)(unsigned new_size);
typedef void (__fastcall * draw_text_type)(wchar_t const * text, int x, int y, unsigned colour, int unknown);
typedef unsigned (__fastcall * get_text_width_type)(wchar_t const * text, unsigned * width, unsigned * file_number);
typedef void (__stdcall * draw_line_type)(int start_x, int start_y, int end_x, int end_y, unsigned colour, unsigned unknown);
typedef unsigned (__stdcall * get_unit_stat_type)(unit * unit_pointer, unsigned stat, unsigned stat2);
typedef unit * (__stdcall * get_player_unit_type)();
typedef uchar (__stdcall * get_difficulty_type)();

extern set_font_size_type d2_set_font_size;
extern draw_text_type d2_draw_text;
extern get_text_width_type d2_get_text_width;
extern draw_line_type d2_draw_line;
extern get_unit_stat_type d2_get_unit_stat;
extern get_player_unit_type d2_get_player_unit;
extern get_difficulty_type d2_get_difficulty;

void draw_text(std::string const & text, int x, int y, unsigned colour, bool centered);
void __stdcall draw_box(int x, int y, unsigned colour);
bool get_life(unsigned & current_life, unsigned & maximum_life);
