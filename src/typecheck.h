#pragma once

#include "ast.h"

void
typecheck_stmts(std::list<std::unique_ptr<StmtNode>> &stmts,
                std::optional<Type> fun_ret_type = std::nullopt);