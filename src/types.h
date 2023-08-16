#pragma once

#include <string>

typedef enum { IntType, StringType, CharType, VoidType } Type;

Type        str_to_type(std::string &type_str);
std::string type_to_str(Type type);