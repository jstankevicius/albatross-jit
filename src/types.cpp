#include "types.h"

Type
str_to_type(std::string &type_str)
{
    if (type_str == "int")
        return Type::Int;
    else if (type_str == "string")
        return Type::String;
    else if (type_str == "void")
        return Type::Void;
    else if (type_str == "char")
        return Type::Char;
    else {
        printf("Invalid type %s\n", type_str.c_str());
        exit(-1);
    }
}

std::string
type_to_str(Type type)
{
    switch (type) {
    case Type::Int: return "int";
    case Type::String: return "string";
    case Type::Void: return "void";
    case Type::Char: return "char";
    }
}