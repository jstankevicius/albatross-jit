#pragma once

#include "ast.h"
#include "symtab.h"

void resolve_stmts(std::vector<StmtNode_p> &stmts, SymbolTable &st);