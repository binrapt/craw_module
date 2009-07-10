#pragma once

#include <string>
#include <ail/types.hpp>

bool hash_d2key(std::string const & cdkey, ulong client_token, ulong server_token, std::string & output, std::string & public_value);
