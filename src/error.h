#pragma once

#include <memory>

#include "token.h"

#define EXIT_LEXER_FAILURE 201
#define EXIT_PARSER_FAILURE 202
#define EXIT_SYMRES_FAILURE 203
#define EXIT_TYPECHECK_FAILURE 204
#define EXIT_RUNTIME_FAILURE 205

[[noreturn]] void err_token(std::shared_ptr<Token> token, std::string message);
void print_err(const std::string &src, int line_num,
               const std::string &message);