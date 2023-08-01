#pragma once

#include "ast.h"
#include "symtab.h"
#include "types.h"

void resolve_stmts(std::vector<StmtNode_p> &stmts, SymbolTable<VarInfo> &vars,
                   SymbolTable<FunInfo> &functions);