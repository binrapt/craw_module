#include <ail/string.hpp>
#include "debug_registers.hpp"
#include "utility.hpp"

void set_debug_registers(CONTEXT & thread_context, unsigned address)
{
	std::vector<unsigned> addresses;
	addresses.push_back(address);
	set_debug_registers(thread_context, addresses);
}

void set_debug_registers(CONTEXT & thread_context, std::vector<unsigned> & addresses)
{
	DWORD * dr_addresses[] =
	{
		&thread_context.Dr0,
		&thread_context.Dr1,
		&thread_context.Dr2,
		&thread_context.Dr3
	};

	thread_context.Dr6 = 0;
	thread_context.Dr7 = 0;

	thread_context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	for(std::size_t i = 0; i < 4 && i < addresses.size(); i++)
	{
		*(dr_addresses[i]) = addresses[i];
		thread_context.Dr7 |= (0 << (4 * i + 16)) | (1 << (2 * i));
	}

	for(std::size_t i = addresses.size(); i < 4; i++)
		*(dr_addresses[i]) = 0;
}

void print_debug_registers(CONTEXT & thread_context)
{
	write_line("DR0: " + ail::hex_string_32(thread_context.Dr0));
	write_line("DR1: " + ail::hex_string_32(thread_context.Dr1));
	write_line("DR2: " + ail::hex_string_32(thread_context.Dr2));
	write_line("DR3: " + ail::hex_string_32(thread_context.Dr3));
	write_line("DR7: " + ail::hex_string_32(thread_context.Dr7));
}
