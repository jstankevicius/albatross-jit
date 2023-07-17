#pragma once

#include <memory>

#include "token.h"

#define EXIT_LEXER_FAILURE 201
#define EXIT_PARSER_FAILURE 202
#define EXIT_SYMRES_FAILURE 203
#define EXIT_TYPECHECK_FAILURE 204
#define EXIT_RUNTIME_FAILURE 205


class AlbatrossError : public std::runtime_error {

private:
  int line;
  int col;

public:
  AlbatrossError(const std::string &msg, int line_num, int col_num)
      : std::runtime_error(msg), line(line_num), col(col_num) {}

  inline int line_num() { return line; }
  inline int col_num() { return col; }
};

void print_err(const std::string &src, int line_num, int col_num,
               const std::string &message);