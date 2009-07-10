#include <iostream>
#include <string>
#include <cstdio>
#include <boost/foreach.hpp>
#include <ail/string.hpp>
#include <ail/memory.hpp>
#include <ail/file.hpp>
#include "patch.hpp"
#include "arguments.hpp"
#include "utility.hpp"
#include "debug_registers.hpp"
#include "d2_cdkey.hpp"

typedef HWND (WINAPI * FindWindow_type)(LPCTSTR lpClassName, LPCTSTR lpWindowName);
typedef HANDLE (WINAPI * CreateFile_type)(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE (WINAPI * CreateThread_type)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef HMODULE (WINAPI * LoadLibrary_type)(LPCTSTR lpFileName);
typedef HWND (WINAPI * CreateWindowEx_type)(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef int (WINAPI * recv_type)(SOCKET s, char *buf, int len, int flags);
typedef int (WINAPI * send_type)(SOCKET s, const char * buf, int len, int flags);

namespace
{
	FindWindow_type real_FindWindow;
	CreateFile_type real_CreateFile;
	CreateThread_type real_CreateThread;
	LoadLibrary_type real_LoadLibrary;
	CreateWindowEx_type real_CreateWindowEx;
	recv_type real_recv;
	send_type real_send;

	ulong server_token;
}

HWND WINAPI patched_FindWindow(LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
	if(std::string(lpClassName) == "Diablo II")
		return NULL;
	else
		return real_FindWindow(lpClassName, lpWindowName);
}

HANDLE WINAPI patched_CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	std::string path(lpFileName);

	std::string file_name;
	std::size_t offset = path.rfind('\\');
	if(offset == std::string::npos)
	{
		offset = 0;
		file_name = path;
	}
	else
	{
		offset++;
		file_name = path.substr(offset);
	}

	std::string fixed_path;

	if(file_name == "bncache.dat")
		fixed_path = path.substr(0, offset) + "bncache\\" + ail::number_to_string<DWORD>(GetCurrentProcessId()) + ".dat";
	else
		fixed_path = path;
	return real_CreateFile(fixed_path.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE WINAPI patched_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	unsigned return_address;

	__asm
	{
		mov eax, [esp]
		mov return_address, eax
	}

	DWORD thread_id;
	dwCreationFlags |= CREATE_SUSPENDED;
	HANDLE output = real_CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, &thread_id);
	if(verbose)
		write_line("New thread " + ail::hex_string_32(thread_id) + " at " + ail::hex_string_32(reinterpret_cast<unsigned>(lpStartAddress)) + " created by " + ail::hex_string_32(return_address));
	if(lpThreadId)
		*lpThreadId = thread_id;

	CONTEXT thread_context;
	thread_context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if(GetThreadContext(output, &thread_context))
	{
		unsigned thread_target = reinterpret_cast<unsigned>(lpStartAddress);
		perform_debug_register_trigger_check(output, thread_context, thread_target);
	}
	else
		write_line("Failed to retrieve thread context!");

	if(ResumeThread(output) == -1)
		write_line("Failed to resume thread!");

	return output;
}

HMODULE WINAPI patched_LoadLibrary(LPCTSTR lpFileName)
{
	HMODULE output = real_LoadLibrary(lpFileName);
	std::string file_name = lpFileName;
	if(verbose)
		write_line("Loaded DLL " + file_name + ": " + ail::hex_string_32(reinterpret_cast<unsigned>(output)));
	perform_dll_check();
	return output;
}

std::string get_d2_window_name()
{
	if(incremental_window_title)
	{
		unsigned i = 0;
		while(true)
		{
			std::string name = "D" + ail::number_to_string(i);
			if(!real_FindWindow(0, name.c_str()))
				return name;
			i++;
		}
	}
	else
		return window_title;
}

HWND WINAPI patched_CreateWindowEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	std::string name = lpWindowName;
	if(name == "Diablo II")
	{
		name = get_d2_window_name();
		if(verbose)
			write_line("Using window name " + name);
	}
	HWND output = real_CreateWindowEx(dwExStyle, lpClassName, name.c_str(), dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	if(verbose)
		write_line("CreateWindowEx retruned " + ail::hex_string_32(reinterpret_cast<unsigned>(output)));
	if(output == 0)
		write_line("CreateWindowEx failed with error number " + ail::hex_string_32(GetLastError()));
	return output;
}

int WINAPI patched_recv(SOCKET s, char * buf, int len, int flags)
{
	int output = real_recv(s, buf, len, flags);
	if(output != SOCKET_ERROR)
	{
		std::string data(buf, output);
		if(data.size() > 20 && data.substr(0, 2) == "\xff\x25" && data.substr(8, 2) == "\xff\x50")
		{
			server_token = ail::read_little_endian(data.c_str(), 4, 16);
			if(verbose)
				write_line("Retrieved server token: " + ail::hex_string_32(server_token));
		}
	}
	return output;
}

int WINAPI patched_send(SOCKET s, const char * buf, int len, int flags)
{
	std::string data(buf, len);
	if(use_custom_keys && data.size() > 64 && data.substr(0, 2) == "\xff\x51")
	{
		ulong client_token = ail::read_little_endian(data.c_str(), 4, 4);
		std::string
			classic_hash,
			classic_public,
			expansion_hash,
			expansion_public;

		std::string original_packet = ail::hex_string(data);

		if(!hash_d2key(classic_key, client_token, server_token, classic_hash, classic_public))
		{
			error("Invalid Diablo II classic key!");
			exit_process();
		}

		if(!hash_d2key(expansion_key, client_token, server_token, expansion_hash, expansion_public))
		{
			error("Invalid Diablo II expansion key!");
			exit_process();
		}

		if(verbose)
			write_line("Replacing the CD keys in the outgoing packet");

		data.replace(32, 4, classic_public);
		data.replace(40, 20, classic_hash);

		data.replace(68, 4, expansion_public);
		data.replace(76, 20, expansion_hash);

		return real_send(s, data.c_str(), static_cast<int>(data.size()), flags);
	}
	else
		return real_send(s, buf, len, flags);
}

bool apply_hot_patches()
{
	std::string const
		kernel = "kernel32.dll",
		user = "user32.dll",
		winsock = "wsock32.dll",
		winsock2 = "ws2_32.dll";

	hot_patch_entry patches[] =
	{
		hot_patch_entry(user, "FindWindowA", &patched_FindWindow, reinterpret_cast<void * &>(real_FindWindow)),
		hot_patch_entry(user, "CreateWindowExA", &patched_CreateWindowEx, reinterpret_cast<void * &>(real_CreateWindowEx)),
		hot_patch_entry(kernel, "CreateFileA", &patched_CreateFile, reinterpret_cast<void * &>(real_CreateFile)),
		hot_patch_entry(kernel, "CreateThread", &patched_CreateThread, reinterpret_cast<void * &>(real_CreateThread)),
		hot_patch_entry(kernel, "LoadLibraryA", &patched_LoadLibrary, reinterpret_cast<void * &>(real_LoadLibrary)),
		hot_patch_entry(winsock, "recv", &patched_recv, reinterpret_cast<void * &>(real_recv)),
		hot_patch_entry(winsock2, "send", &patched_send, reinterpret_cast<void * &>(real_send)),
	};

	BOOST_FOREACH(hot_patch_entry & entry, patches)
	{
		if(!hot_patch_function(entry.module, entry.procedure, entry.function, entry.real_function))
			return false;
	}

	return true;
}
