#include <iostream>
#include <windows.h>
#include <ail/string.hpp>
#include "utility.hpp"
#include "arguments.hpp"
#include "console.hpp"

unsigned page_size = 0x1000;

namespace
{
	HANDLE console_handle = 0;
}

void error(std::string const & message)
{
	MessageBox(0, message.c_str(), "Error", MB_OK);
}

void initialise_console()
{
	if(console_handle == 0)
	{
		AllocConsole();
		console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
		std::cout.sync_with_stdio();
		std::cin.sync_with_stdio();

		if(prompt_mode)
			launch_prompt();
	}
}

void write_text(std::string const & text)
{
	if(console_output)
	{
		initialise_console();

		DWORD bytes_written;
		WriteConsole(console_handle, reinterpret_cast<void const *>(text.c_str()), static_cast<DWORD>(text.size()), &bytes_written, 0);
	}
}

void write_line(std::string const & text)
{
	write_text(text + "\n");
}

bool procedure_lookup(std::string const & module, std::string const & procedure, void * & output)
{
	HMODULE module_handle = GetModuleHandle(module.c_str());
	if(module_handle == 0)
	{
		write_line("Unable to get the module handle of " + module);
		return false;
	}

	output = reinterpret_cast<void *>(GetProcAddress(module_handle, procedure.c_str()));
	if(output == 0)
	{
		write_line("Unable to retrieve the address of " + procedure + " in " + module);
		return false;
	}

	return true;
}

unsigned get_page(unsigned address)
{
	return address & ~(page_size - 1);
}

std::string number_to_string(ulong input, ulong size)
{
	std::string output;
	for(ulong i = 0; i < size; i++)
		output.push_back(static_cast<char>((input >> (i * 8)) & 0xff));
	return output;
}

std::string dword_to_string(ulong input)
{
	return number_to_string(input, 4);
}

void exit_process()
{
	ExitProcess(1);
}
