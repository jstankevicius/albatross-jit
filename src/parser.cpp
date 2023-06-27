#include "parser.h"

#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "token.h"

ExpNode_p parse_expression(std::deque<std::shared_ptr<Token>>& tokens) {

}

// Expect the next token in the stream to have a particular string as its
// contents. If not, fail with an error on the token.
void expect_token_string(std::string str,
                         std::deque<std::shared_ptr<Token>> &tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
  }

  auto &token = tokens.front();
  if (token->string_value != str) {
    err_token(token, "syntax error: expected '" + str + "', but got '" +
                         token->string_value + "' ");
  }

  tokens.pop_front();
}

// Expect the next token in the stream to have a particular type. If not, fail
// with an error on the token.
void expect_token_type(TokenType type,
                       std::deque<std::shared_ptr<Token>> &tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
  }

  switch (type) {
  default:
    break;
  }

  auto token = tokens.front();

  tokens.pop_front();
}

ExpNode_p parse_int_expr(std::deque<std::shared_ptr<Token>> &tokens) {
  assert(tokens.front()->type == TokenType::IntLiteral);
  int val = std::atoi(tokens.front()->string_value.c_str());
  expect_token_type(TokenType::IntLiteral, tokens);
  printf("Found integer %d\n", val);
  return new_int_node(val);
}

ExpNode_p parse_expr(std::deque<std::shared_ptr<Token>> &tokens) {
  // Maybe this should actually just be the shunting-yard algorithm.
  // Modification:
  // if we see a function, we call parse_expr on its arguments until we
  // reach a closing parenthesis.
  const auto front = tokens.front();
  switch (front->type) {
  case TokenType::IntLiteral:
    return parse_int_expr(tokens);

  default:
    err_token(front, "expected an expression");
  }

  return std::make_shared<ExpNode>();
}

StmtNode_p parse_var(std::deque<std::shared_ptr<Token>> &tokens) {
  auto p = std::make_unique<StmtNode>();
  tokens.pop_front();
  return std::make_unique<StmtNode>();
}

StmtNode_p parse_return(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordReturn, tokens);
  ExpNode_p ret_exp = parse_int_expr(tokens);
  expect_token_type(TokenType::Semicolon, tokens);
  return std::make_shared<StmtNode>();
}

StmtNode_p parse_stmt(std::deque<std::shared_ptr<Token>> &tokens) {
  // Parse a top-level statement and return its AST.
  printf("Parsing a statement\n");
  const auto &front = tokens.front();
  auto p = std::make_shared<StmtNode>();

  switch (front->type) {
  case TokenType::KeywordIf:
    break;
  case TokenType::KeywordWhile:
    break;
  case TokenType::KeywordRepeat:
    break;
  case TokenType::KeywordReturn:
    printf("Parsing return statement\n");
    return parse_return(tokens);
  case TokenType::KeywordVar:
    break;
  case TokenType::KeywordFun:
    break;
  case TokenType::Identifier:
    break;
  default:
    err_token(front, "expected a statement");
  }

  return p;
}