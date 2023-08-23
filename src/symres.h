#pragma once

#include "ast.h"
#include "symtab.h"
#include "types.h"

void
resolve_stmts(std::vector<std::unique_ptr<StmtNode>> &stmts,
              SymbolTable<VarInfo>                   &vars,
              SymbolTable<FunInfo>                   &functions);