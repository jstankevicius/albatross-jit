#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

std::shared_ptr<StmtNode>
parse_stmt(std::deque<std::shared_ptr<Token>> &tokens);
