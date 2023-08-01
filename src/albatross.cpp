#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compiler_stages.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "symres.h"
#include "typecheck.h"

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

  try {

#ifdef COMPILE_STAGE_LEXER
    ProgramText text(content);

    auto tokens = tokenize(text);

#ifdef COMPILE_STAGE_PARSER
    auto stmts = parse_stmts(tokens);

#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
    SymbolTable<VarInfo> vars;
    SymbolTable<FunInfo> functions;

    functions.enter_scope();
    vars.enter_scope();

    resolve_stmts(stmts, vars, functions);
#ifdef COMPILE_STAGE_TYPE_CHECKER
    typecheck_stmts(stmts);
#endif
#endif
#endif
#endif
  } catch (AlbatrossError &e) {
    print_err(content, e.line_num(), e.col_num(), e.what());
    exit(e.exit_code());
  }
}