#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

std::shared_ptr<Token>
expect_any_token(std::deque<std::shared_ptr<Token>> &tokens);
std::shared_ptr<Token>
expect_token_type(TokenType type, std::deque<std::shared_ptr<Token>> &tokens);
ExpNode_p parse_int_expr(std::deque<std::shared_ptr<Token>> &tokens);

std::shared_ptr<StmtNode>
parse_stmt(std::deque<std::shared_ptr<Token>> &tokens);

std::vector<std::shared_ptr<StmtNode>>
parse_stmts(std::deque<std::shared_ptr<Token>> &tokens);
