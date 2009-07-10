#pragma once

#include <string>

extern bool
	console_output,
	incremental_window_title,
	verbose,

	use_custom_keys,

	prompt_mode;

extern std::string
	window_title,

	classic_key,
	expansion_key,

	d2_arguments,

	python_script;

void process_command_line();
