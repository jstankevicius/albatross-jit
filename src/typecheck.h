#pragma once

#include "ast.h"

void typecheck_stmts(std::vector<StmtNode_p> &stmts,
                     std::optional<Type>      fun_ret_type = std::nullopt);