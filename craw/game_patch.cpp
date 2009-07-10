#include <vector>
#include <ail/string.hpp>
#include <boost/foreach.hpp>
#include <windows.h>
#include <tlhelp32.h>
#include "patch.hpp"
#include "utility.hpp"
#include "debug_registers.hpp"
#include "automap.hpp"
#include "arguments.hpp"
#include "packet_handler.hpp"
#include "d2_functions.hpp"

namespace
{
	unsigned original_game_packet_handler;
	DWORD main_thread_id;

	unsigned light_handler_address = 0x6FB0F8F0u;

	unsigned automap_handler_address = 0x6FAEF920u;
	unsigned automap_loop_address = 0x6FAF0350u;
}

typedef void (* initialisation_function_type)(unsigned base);
typedef void (* debug_register_event_handler_type)(CONTEXT & thread_context);

struct dll_load_event
{
	bool is_loaded;
	std::string dll_name;
	initialisation_function_type initialisation_function;

	dll_load_event()
	{
	}

	dll_load_event(std::string const & dll_name, initialisation_function_type initialisation_function):
		is_loaded(false),
		dll_name(dll_name),
		initialisation_function(initialisation_function)
	{
	}
};

struct debug_register_entry
{
	unsigned address;
	debug_register_event_handler_type handler;

	debug_register_entry()
	{
	}

	debug_register_entry(unsigned address, debug_register_event_handler_type handler):
		address(address),
		handler(handler)
	{
	}
};

struct debug_register_trigger
{
	std::string description;
	unsigned thread_target;
	std::vector<unsigned> target_addresses;

	debug_register_trigger()
	{
	}

	debug_register_trigger(std::string const & description, unsigned thread_target, unsigned target_address1):
		description(description),
		thread_target(thread_target)
	{
		target_addresses.push_back(target_address1);
	}
};

namespace
{
	std::vector<dll_load_event> dll_vector;
	std::vector<debug_register_entry> debug_register_entries;
	std::vector<debug_register_trigger> debug_register_triggers;
}

void actual_packet_handler(uchar * data, unsigned size)
{
	//write_line("Got a " + ail::hex_string_8(data[0]) + " packet (" + ail::number_to_string<unsigned>(size) + " byte(s))");
	//write_line(ail::hex_string(std::string(reinterpret_cast<char *>(data), static_cast<std::size_t>(size))));
}

void __declspec(naked) custom_packet_handler()
{
	__asm
	{
		pushad
		push ebx
		push ecx
		call actual_packet_handler
		add esp, 8
		popad
		sub esp, 0110h
		mov eax, original_game_packet_handler
		add eax, 6
		jmp eax
	}
}

void __declspec(naked) custom_light_handler()
{
	__asm
	{
		push ecx
		test eax, eax
		mov byte ptr [esp], 0
		mov byte ptr [esp + 1], 0
		mov byte ptr [esp + 2], 0
		jz other_branch
		mov eax, 0ffh
		mov byte ptr [esp], al
		mov byte ptr [esp + 1], al
		mov byte ptr [esp + 2], al
		mov ecx, light_handler_address
		add ecx, 08eh
		jmp ecx

	other_branch:
		mov eax, light_handler_address
		add eax, 0a5h
		jmp eax
	}
}

void debug_register_packet_handler(CONTEXT & thread_context)
{
	//thread_context.Eip = reinterpret_cast<DWORD>(&custom_packet_handler);
	thread_context.Eip = reinterpret_cast<DWORD>(&main_packet_handler);
}

void debug_register_light(CONTEXT & thread_context)
{
	thread_context.Eip = reinterpret_cast<DWORD>(&custom_light_handler);
	//write_line("Custom light handler!");
}

void debug_register_automap(CONTEXT & thread_context)
{
	thread_context.Eip = reinterpret_cast<DWORD>(&automap_blobs);
}

void debug_register_automap_loop(CONTEXT & thread_context)
{
	thread_context.Eip = reinterpret_cast<DWORD>(&automap_loop);
}

void d2net(unsigned base)
{
	original_game_packet_handler = base + 0x6650;
	debug_register_entries.push_back(debug_register_entry(original_game_packet_handler, &debug_register_packet_handler));

	debug_register_triggers.push_back(debug_register_trigger("Game packet handler thread", base + 0x6bd0, original_game_packet_handler));
}

void d2client(unsigned base)
{
	debug_register_entries.push_back(debug_register_entry(light_handler_address, &debug_register_light));
	//debug_register_entries.push_back(debug_register_entry(automap_handler_address, &debug_register_automap));
	debug_register_entries.push_back(debug_register_entry(automap_loop_address, &debug_register_automap_loop));

	//patch_address(automap_handler_address, &automap_blobs);
	//write_line("Automap code address: " + ail::hex_string_32(reinterpret_cast<ulong>(&automap_blobs)));

	initialise_d2client_functions(base);
}

void initialise_dll_vector()
{
	dll_vector.push_back(dll_load_event("D2Win.dll", &initialise_d2win_functions));
	dll_vector.push_back(dll_load_event("D2Gfx.dll", &initialise_d2gfx_functions));
	dll_vector.push_back(dll_load_event("D2Common.dll", &initialise_d2common_functions));
	dll_vector.push_back(dll_load_event("D2Client.dll", &d2client));

	dll_vector.push_back(dll_load_event("D2Net.dll", &d2net));
}

void perform_dll_check()
{
	BOOST_FOREACH(dll_load_event & handler, dll_vector)
	{
		if(!handler.is_loaded)
		{
			unsigned base = reinterpret_cast<unsigned>(GetModuleHandle(handler.dll_name.c_str()));
			if(base != 0)
			{
				handler.is_loaded = true;
				if(verbose)
					write_line("Preparing the debug register interception for " + handler.dll_name + " (" + ail::hex_string_32(base) + ")");
				handler.initialisation_function(base);
			}
		}
	}
}

bool perform_debug_register_check(CONTEXT & thread_context)
{
	//write_line("Trying to match the address to a debug register handler");

	unsigned address = static_cast<unsigned>(thread_context.Eip);
	BOOST_FOREACH(debug_register_entry & entry, debug_register_entries)
	{
		if(entry.address != address)
			continue;

		//write_line("Discovered a debug register handler for address " + ail::hex_string_32(address));
		entry.handler(thread_context);
		return true;
	}

	error("Was unable to find a debug register handler for address " + ail::hex_string_32(address));
	return false;
}

bool perform_debug_register_trigger_check(HANDLE thread_handle, CONTEXT & thread_context, unsigned thread_target)
{
	BOOST_FOREACH(debug_register_trigger & trigger, debug_register_triggers)
	{
		if(trigger.thread_target != thread_target)
			continue;

		set_debug_registers(thread_context, trigger.target_addresses);
		if(!SetThreadContext(thread_handle, &thread_context))
		{
			error("Failed to set thread context for target " + ail::hex_string_32(thread_target) + "!");
			return false;
		}

		if(verbose)
			write_line("Important thread detected: " + trigger.description);
		return true;
	}
	//write_line("This is not a relevant thread.");
	return false;
}

void hook_main_thread()
{
	HANDLE thread_handle = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, 0, main_thread_id);
	CONTEXT thread_context;
	thread_context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if(!GetThreadContext(thread_handle, &thread_context))
	{
		error("Unable to retrieve main thread context: " + ail::hex_string_32(GetLastError()));
		CloseHandle(thread_handle);
		return;
	}

	std::vector<unsigned> addresses;
	addresses.push_back(light_handler_address);
	//addresses.push_back(automap_handler_address);
	addresses.push_back(automap_loop_address);

	set_debug_registers(thread_context, addresses);

	if(!SetThreadContext(thread_handle, &thread_context))
	{
		error("Failed to set main thread context: " + ail::hex_string_32(GetLastError()));
		CloseHandle(thread_handle);
		return;
	}

	CloseHandle(thread_handle);

	if(verbose)
		write_line("Changed the debug registers of the main thread " + ail::hex_string_32(main_thread_id));
}

bool process_thread_entry(DWORD process_id, DWORD current_thread_id, THREADENTRY32 & thread_entry)
{
	if(thread_entry.th32OwnerProcessID != process_id)
		return false;
	DWORD thread_id = thread_entry.th32ThreadID;
	if(thread_id == current_thread_id)
		return false;

	if(verbose)
		write_line("Main thread: " + ail::hex_string_32(thread_id));
	main_thread_id = thread_id;
	hook_main_thread();
	return true;
}

bool process_main_thread()
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(snapshot == INVALID_HANDLE_VALUE)
	{
		error("CreateToolhelp32Snapshot failed: " + ail::hex_string_32(GetLastError()));
		CloseHandle(snapshot);
		return false;
	}
	DWORD process_id = GetCurrentProcessId();
	DWORD current_thread_id = GetCurrentThreadId();
	THREADENTRY32 thread_entry;
	thread_entry.dwSize = sizeof(THREADENTRY32);
	if(!Thread32First(snapshot, &thread_entry))
	if(snapshot == INVALID_HANDLE_VALUE)
	{
		error("Thread32First failed: " + ail::hex_string_32(GetLastError()));
		CloseHandle(snapshot);
		return false;
	}
	bool success = process_thread_entry(process_id, current_thread_id, thread_entry);
	while(Thread32Next(snapshot, &thread_entry))
		success = success || process_thread_entry(process_id, current_thread_id, thread_entry);
	if(!success)
	{
		error("Unable to detect main thread!");
		CloseHandle(snapshot);
		return false;
	}
	CloseHandle(snapshot);
	return true;
}
