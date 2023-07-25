#include "types.h"

Type str_to_type(std::string &type_str) {
  if (type_str == "int")
    return Type::IntType;
  else if (type_str == "string")
    return Type::StringType;
  else if (type_str == "void")
    return Type::VoidType;
  else if (type_str == "char")
    return Type::CharType;
  else {
    printf("Invalid type %s\n", type_str.c_str());
    exit(-1);
  }
}

std::string type_to_str(Type type) {
  switch (type) {
  case Type::IntType:
    return "int";
  case Type::StringType:
    return "string";
  case Type::VoidType:
    return "void";
  case Type::CharType:
    return "char";
  }
}