#pragma once

#include <memory>

#include "token.h"

[[noreturn]] void err_token(std::shared_ptr<Token> token, std::string message);
void print_err(const std::string &src, int line_num,
               const std::string &message);