#pragma once

#include <string>
#include <ail/types.hpp>
#include <windows.h>

extern unsigned page_size;

void error(std::string const & message);
void initialise_console();
void write_text(std::string const & text);
void write_line(std::string const & text);
bool procedure_lookup(std::string const & module, std::string const & procedure, void * & output);
unsigned get_page(unsigned address);
void exit_process();

std::string dword_to_string(ulong input);
