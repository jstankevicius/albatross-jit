#pragma once

#include <memory>

#define EXIT_LEXER_FAILURE (char)201
#define EXIT_PARSER_FAILURE (char)202
#define EXIT_SYMRES_FAILURE (char)203
#define EXIT_TYPECHECK_FAILURE (char)204
#define EXIT_RUNTIME_FAILURE (char)205

class AlbatrossError : public std::runtime_error {

private:
  int _line_num;
  int _col_num;
  char _exit_code;

public:
  AlbatrossError(const std::string &msg, int line_num, int col_num,
                 char exit_code)
      : std::runtime_error(msg), _line_num(line_num), _col_num(col_num),
        _exit_code(exit_code) {}

  inline int line_num() { return _line_num; }
  inline int col_num() { return _col_num; }
  inline char exit_code() { return _exit_code; }
};

void print_err(const std::string &src, int line_num, int col_num,
               const std::string &message);