#pragma once

#include <vector>
#include <windows.h>

void set_debug_registers(CONTEXT & thread_context, unsigned address);
void set_debug_registers(CONTEXT & thread_context, std::vector<unsigned> & addresses);
void print_debug_registers(CONTEXT & thread_context);
