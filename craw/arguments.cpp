#include <windows.h>
#include <cstring>
#include <ail/arguments.hpp>
#include "arguments.hpp"
#include "utility.hpp"

bool
	console_output,
	incremental_window_title,

	verbose,

	use_custom_keys,

	prompt_mode;

std::string
	window_title,

	classic_key,
	expansion_key,

	d2_arguments,

	python_script;

namespace
{
	char * command_line_arguments;
}

std::string key_string(std::string const & key)
{
	if(key.empty())
		return "Key has not been specified";
	else
		return "A custom key has been specified";
}

bool install_command_line_patch()
{
	d2_arguments = " " + d2_arguments;
	command_line_arguments = new char[d2_arguments.size() + 1];
	std::strcpy(command_line_arguments, d2_arguments.c_str());

	void * address;
	if(!procedure_lookup("kernel32.dll", "GetCommandLineA", address))
	{
		error("Unable to look up the address for the command line patch");
		return false;
	}

	__asm
	{
		mov eax, address
		movzx ebx, byte ptr [eax]
		cmp ebx, 0a1h
		jnz failure
		mov eax, dword ptr [eax + 1]
		mov ebx, command_line_arguments
		mov dword ptr [eax], ebx
	}

	if(verbose)
		write_line("Successfully installed the command line patch");
	
	return true;

failure:
	error("The format of the GetCommandlineA procedure in your kernel32.dll is unknown to the module's patcher and cannot be patched");
	return false;
}

void process_command_line()
{
	ail::argument_parser argument_parser;

	argument_parser.flag("console_output", console_output).default_flag(true);
	argument_parser.flag("incremental_window_title", incremental_window_title).default_flag(false);
	argument_parser.flag("verbose", verbose).default_flag(false);
	argument_parser.flag("prompt", prompt_mode).default_flag(false);

	argument_parser.string("window_title", window_title).default_string("Diablo II");

	argument_parser.string("classic_key", classic_key);
	argument_parser.string("expansion_key", expansion_key);

	argument_parser.string("d2_arguments", d2_arguments).default_string("-w");

	argument_parser.string("python_script", python_script);

	try
	{
		argument_parser.parse(GetCommandLine());

		if(verbose)
		{
			write_line("Window title: " + window_title);
			write_line("Classic key: " + key_string(classic_key));
			write_line("Expansion key: " + key_string(expansion_key));
		}

		use_custom_keys = !classic_key.empty() && !expansion_key.empty();

		if(verbose)
		{
			if(use_custom_keys)
				write_line("Using the specified custom keys");
			else
				write_line("Not using a custom pair of keys");
		}

		install_command_line_patch();
	}
	catch(ail::exception & exception)
	{
		error("Argument parser error: " + exception.get_message());
	}
}
