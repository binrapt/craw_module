#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <ail/string.hpp>
#include <windows.h>
#include "console.hpp"
#include "d2_functions.hpp"

namespace
{
	console_command commands[] =
	{
		console_command("help", "", "Prints this help", 0, &print_help),
		console_command("?", "", "See 'help'", 0, &print_help),
		console_command("quit", "", "Terminates the program", 0, &quit_program),
		console_command("exit", "", "See 'quit'", 0, &quit_program),
		console_command("life", "", "Print your character's life", 0, &print_life),
	};
}

console_command::console_command(std::string const & command, std::string const & argument_description, std::string const & description, std::size_t argument_count, command_handler handler):
	command(command),
	argument_description(argument_description),
	description(description),
	argument_count(argument_count),
	handler(handler)
{
}

bool console_command::match(std::string const & match_command, string_vector const & arguments) const
{
	return command == match_command && argument_count == arguments.size();
}
void print_life(string_vector const & arguments)
{
	unsigned
		current_life,
		maximum_life;

	if(get_life(current_life, maximum_life))
		std::cout << "Life: " << current_life << "/" << maximum_life << std::endl;
	else
		std::cout << "Your character is not in a game" << std::endl;
}

void print_help(string_vector const & arguments)
{
	std::cout << "Available commands:" << std::endl;
	BOOST_FOREACH(console_command const & current_command, commands)
	{
		std::cout << std::endl << "Command: " << current_command.command << " " << current_command.argument_description << std::endl;
		std::cout << "Description: " << current_command.description << std::endl;
	}
}

void quit_program(string_vector const & arguments)
{
	std::cout << "Terminating..." << std::endl;
	ExitProcess(0);
}

void console_prompt()
{
	while(true)
	{
		std::cout << ">> ";
		std::string line;
		std::getline(std::cin, line);
		if(line.empty())
			continue;
		string_vector tokens = ail::tokenise(line, " ");
		if(tokens.empty())
			continue;

		std::string const & command = tokens[0];

		bool success = false;

		string_vector arguments = tokens;
		arguments.erase(arguments.begin());

		BOOST_FOREACH(console_command const & current_command, commands)
		{
			if(current_command.match(command, arguments))
			{
				current_command.handler(arguments);
				success = true;
				break;
			}
		}

		if(!success)
			std::cout << "Unknown command: \"" << command << "\"" << std::endl;
	}
}

void launch_prompt()
{
	//dirty!
	new boost::thread(&console_prompt);
}
