#include "ast.h"
#include <iostream>
#include <vector>

bool
fold_stmts(std::vector<std::unique_ptr<StmtNode>> &stmts);