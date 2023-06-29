#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

void expect_any_token(std::deque<std::shared_ptr<Token>>& tokens);
void expect_token_type(TokenType type, std::deque<std::shared_ptr<Token>>& tokens);
ExpNode_p parse_int_expr(std::deque<std::shared_ptr<Token>> &tokens);

std::shared_ptr<StmtNode>
parse_stmt(std::deque<std::shared_ptr<Token>> &tokens);
