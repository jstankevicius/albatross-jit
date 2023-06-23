#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

std::unique_ptr<StmtNode>
parse_stmt_or_var(std::deque<std::shared_ptr<Token>>& tokens);
