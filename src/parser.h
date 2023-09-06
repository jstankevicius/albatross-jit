#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

std::unique_ptr<Token>
expect_any_token(std::deque<std::unique_ptr<Token>> &tokens);
std::unique_ptr<Token>
expect_token_type(TokenType type, std::deque<std::unique_ptr<Token>> &tokens);
std::unique_ptr<ExpNode>
parse_int_exp(std::deque<std::unique_ptr<Token>> &tokens);

std::unique_ptr<ExpNode>
exp_bp(std::deque<std::unique_ptr<Token>> &tokens, int bp);
std::unique_ptr<ExpNode>
parse_exp(std::deque<std::unique_ptr<Token>> &tokens);

std::unique_ptr<StmtNode>
parse_stmt(std::deque<std::unique_ptr<Token>> &tokens);

std::list<std::unique_ptr<StmtNode>>
parse_stmts(std::deque<std::unique_ptr<Token>> &tokens);
