#pragma once

#include <string>

enum class Type { Int, String, Char, Void };

Type
str_to_type(std::string &type_str);

std::string
type_to_str(Type type);