#pragma once

#include <string>

#include <windows.h>

struct hot_patch_entry
{
	std::string
		module,
		procedure;

	void * function;
	void * & real_function;

	hot_patch_entry(std::string const & module, std::string const & procedure, void * function, void * & real_function);
};

void initialise_dll_vector();
void perform_dll_check();
bool perform_debug_register_check(CONTEXT & thread_context);
bool perform_debug_register_trigger_check(HANDLE thread_handle, CONTEXT & thread_context, unsigned thread_target);
bool process_main_thread();

bool hot_patch_function(std::string const & module, std::string const & function, void * function_pointer, void * & old_address);
bool patch_function(std::string const & module, std::string const & procedure, void * & address, void * custom_function);
bool patch_address(unsigned address, void * target);

bool apply_hot_patches();
bool patch_exception_handler();
