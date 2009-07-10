#include <boost/foreach.hpp>
#include <ail/memory.hpp>
#include <ail/string.hpp>
#include "patch.hpp"
#include "utility.hpp"
#include "arguments.hpp"

hot_patch_entry::hot_patch_entry(std::string const & module, std::string const & procedure, void * function, void * & real_function):
	module(module),
	procedure(procedure),
	function(function),
	real_function(real_function)
{
}

bool hot_patch_function(std::string const & module, std::string const & function, void * function_pointer, void * & old_address)
{
	HMODULE module_handle = GetModuleHandle(module.c_str());
	if(module_handle == 0)
	{
		error("Unable to retrieve " + module + " module handle");
		return false;
	}

	char * address = reinterpret_cast<char *>(GetProcAddress(module_handle, function.c_str()));
	if(address == NULL)
	{
		error("Unable to get procedure address of " + function + " in module " + module);
		return false;
	}

	old_address = address + 2;

	std::string expected_bytes = "\x8b\xff";
	std::string procedure_bytes;
	procedure_bytes.assign(address, expected_bytes.length());

	if(procedure_bytes != expected_bytes)
	{
		error("Unable to patch " + function + " in module " + module + " - are you running a pre Windows XP SP1 operating system by Microsoft?");
		return false;
	}

	unsigned function_address = reinterpret_cast<unsigned>(function_pointer) - reinterpret_cast<unsigned>(address);
	std::string replacement_string = ail::little_endian_string(function_address, 4);
	std::string replacement = "\xe9" + replacement_string + "\xeb\xf9";

	char * offset = address - 5;

	DWORD old_protection;
	if(VirtualProtect(offset, replacement.length(), PAGE_EXECUTE_READWRITE, &old_protection) == 0)
	{
		error("Unable to patch " + function + " in module " + module + " because I was unable to make it writable");
		return false;
	}

	std::memcpy(offset, replacement.c_str(), replacement.length());

	DWORD unused;
	if(VirtualProtect(offset, replacement.length(), old_protection, &unused) == 0)
	{
		error("Unable to protect our patch of " + function + " in module " + module);
		return false;
	}

	return true;
}

bool patch_function(std::string const & module, std::string const & procedure, void * & address, void * custom_function)
{
	std::string name = module + "!" + procedure;
	if(!procedure_lookup(module, procedure, address))
		return false;

	unsigned const patch_size = 5 + 5;
	//unsigned const patch_size = 5;

	DWORD old_protection;
	if(!VirtualProtect(address, patch_size, PAGE_EXECUTE_READWRITE, &old_protection))
	{
		error("Failed to make the page of " + name + " writable");
		false;
	}

	unsigned eip = reinterpret_cast<unsigned>(address) + patch_size;
	unsigned offset = reinterpret_cast<unsigned>(custom_function) - eip;

	char * data = reinterpret_cast<char *>(address);

	//lea esi, [esi + 0]
	data[0] = '\x8d';
	data[1] = '\x76';
	data[2] = '\x00';

	//mov esi, esi
	data[3] = '\x89';
	data[4] = '\xf6';

	//call address
	data[5] = '\xe8';
	unsigned * offset_pointer = reinterpret_cast<unsigned *>(data + 6);

	/*
	data[0] = '\xe9';
	unsigned * offset_pointer = reinterpret_cast<unsigned *>(data + 1);
	*/

	*offset_pointer = offset;

	DWORD unused;
	if(!VirtualProtect(address, patch_size, old_protection, &unused))
	{
		error("Failed to restore the old page permissions for " + name);
		return false;
	}

	return true;
}

//dangerous, this is detected by Warden due to 0xe9 and possibly the 4 address bytes! Use for singleplayer testing only!

bool patch_address(unsigned address, void * target)
{
	std::size_t patch_size = 5;

	LPVOID address_pointer = reinterpret_cast<LPVOID>(address);

	DWORD old_protection;
	if(!VirtualProtect(address_pointer, patch_size, PAGE_EXECUTE_READWRITE, &old_protection))
	{
		error("Failed to patch address " + ail::hex_string_32(address));
		false;
	}

	unsigned eip = address + static_cast<unsigned>(patch_size);
	*reinterpret_cast<uchar *>(address) = 0xe9;
	*reinterpret_cast<unsigned *>(address + 1) = reinterpret_cast<unsigned>(target) - eip;

	DWORD unused;
	if(!VirtualProtect(address_pointer, patch_size, old_protection, &unused))
	{
		error("Failed to restore the old page permissions for address " + ail::hex_string_32(address));
		return false;
	}

	return true;
}
