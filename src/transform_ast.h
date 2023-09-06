#include "ast.h"
#include <iostream>
#include <vector>

bool
fold_stmts(std::list<std::unique_ptr<StmtNode>> &stmts);

bool
dce_stmts(std::list<std::unique_ptr<StmtNode>> &stmts);