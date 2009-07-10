#include <iostream>
#include <windows.h>
#include <ail/string.hpp>
#include <boost/thread.hpp>
#include "utility.hpp"
#include "anti_debugging.hpp"
#include "hide.hpp"
#include "patch.hpp"
#include "exception_handler.hpp"
#include "arguments.hpp"
#include "python.hpp"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		process_command_line();

		void * module_base = reinterpret_cast<void *>(hinstDLL);
		if(verbose)
			write_line("Module base: " + ail::hex_string_32(reinterpret_cast<unsigned>(hinstDLL)));

		initialise_dll_vector();
		anti_debugging();

		if(!
			(
				hide_module(module_base) &&
				apply_hot_patches() &&
				install_exception_handler() &&
				process_main_thread() &&
				python::initialise_python()
			)
		)
		{
			console_output = true;
			write_line("A fatal error occured, the program is not going to continue, please close this window.");
			while(true)
				Sleep(0);
		}

		if(prompt_mode)
			initialise_console();

		LoadLibrary("D2Client.dll");
	}

	return TRUE;
}
