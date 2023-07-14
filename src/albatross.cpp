#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "symres.h"
#include "typecheck.h"

#define COMPILE_STAGE_LEXER
#define COMPILE_STAGE_PARSER
#define COMPILE_STAGE_SYMBOL_RESOLVER
#define COMPILE_STAGE_TYPE_CHECKER


int main(int argc, char *argv[]) {
  if (argc < 2) {
    // This should just switch to interactive mode
    std::cerr << "Error: no input file" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ifstream file;
  file.open(argv[1]);

  if (!file.is_open()) {
    perror("Error: open()");
    exit(EXIT_FAILURE);
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      (std::istreambuf_iterator<char>()));

#ifdef COMPILE_STAGE_LEXER
  ProgramText text(content);

  auto tokens = tokenize(text);

#ifdef COMPILE_STAGE_PARSER
  auto stmts = parse_stmts(tokens);

#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
  SymbolTable st;
  st.enter_scope();
  resolve_stmts(stmts, st);

#ifdef COMPILE_STAGE_TYPE_CHECKER
  typecheck_stmts(stmts);
#endif
#endif
#endif
#endif
}
