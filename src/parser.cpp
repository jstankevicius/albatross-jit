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

struct OpInfo {
  Operator op;

  int l_bp;
  int r_bp;

  // BinOp or UnOp no longer applies :)
  enum { Prefix, Infix, Postfix, Invalid } kind;
};

OpInfo op_binding_power(const std::deque<std::shared_ptr<Token>>& tokens, bool minus_prefix_flag=false) {
  TokenType op = tokens.front()->type;
  switch (op) {
    case TokenType::OpPlus:   return OpInfo{Operator::OpPlus,  5,  6, OpInfo::Infix};
    case TokenType::OpMinus:  return minus_prefix_flag ? OpInfo{Operator::OpMinus, 11, -1, OpInfo::Prefix} : OpInfo{Operator::OpMinus, 5, 6, OpInfo::Infix};
    case TokenType::OpTimes:  return OpInfo{Operator::OpTimes, 7,  8, OpInfo::Infix};
    case TokenType::OpDiv:    return OpInfo{Operator::OpDiv,   7,  8, OpInfo::Infix};
    case TokenType::OpNot:    return OpInfo{Operator::OpNot,  -1, 11, OpInfo::Prefix};
    case TokenType::Lbracket: return OpInfo{Operator::OpSub,  11, -1, OpInfo::Postfix};
    default: return OpInfo{Operator::OpPlus, -1, -1, OpInfo::Invalid };
  }
  printf("how did we even get here????????\n");
  return OpInfo{Operator::OpPlus,  5,  6, OpInfo::Infix};
}

ExpNode_p exp_bp(std::deque<std::shared_ptr<Token>>& tokens, int min_bp) {

  auto front = tokens.front();
  ExpNode_p lhs;
  switch (front->type) {
    case TokenType::IntLiteral: { 
      lhs = parse_int_expr(tokens);
      break;
    };
    // TODO: Add identifiers
    case TokenType::Lparen: {
      printf("encountered lparen\n");
      expect_token_type(TokenType::Lparen, tokens);
      lhs = exp_bp(tokens, 0);
      printf("Expecting an RPAREN now\n");
      expect_token_type(TokenType::Rparen, tokens);
      break;
    }
    // Well, it can pretty much ONLY be a prefix operator.
    case TokenType::OpMinus:
    case TokenType::OpNot: {
      OpInfo info = op_binding_power(tokens, true);
      auto r_bp = front->type == TokenType::OpMinus ? -1 : info.r_bp;

      // Consume the operator; it is guaranteed to be either OpMinus or OpNot
      expect_any_token(tokens);
      auto rhs = exp_bp(tokens, r_bp);

      lhs = new_unop_node(info.op, rhs);
      break;
    }

    default: err_token(front, "Expected literal, identifier, paren, or prefix operator");
  }
  while (1) {
    if (tokens.empty() || tokens.front()->type == TokenType::Semicolon || tokens.front()->type == TokenType::Rparen) {
      break;
    }
    OpInfo info = op_binding_power(tokens);
    if (info.kind == OpInfo::Postfix) {
      int l_bp = info.l_bp;
      if (l_bp < min_bp) {
        break;
      }

      // Consume op token
      expect_any_token(tokens);

      // new_binop_node(op, lhs, rhs);
      // TODO: implement [ operator
      lhs = new_unop_node(info.op, lhs);
      continue;
    }

    if (info.kind == OpInfo::Infix) {
      int l_bp = info.l_bp;
      int r_bp = info.r_bp;
      if (l_bp < min_bp) {
        break;
      } 
      // Consume op token 
      expect_any_token(tokens);

      // Now parse rhs
      auto rhs = exp_bp(tokens, r_bp);
      // TODO: cons rhs and lhs together
      lhs = new_binop_node(info.op, lhs, rhs);
      continue;
    }

    break;
  }

  return lhs;
}

ExpNode_p parse_expression(std::deque<std::shared_ptr<Token>>& tokens) {
  printf("Parsing an expression\n");
  auto e = exp_bp(tokens, 0);
  std::cout << e->to_str() << "\n";
  return e;
}

// Expect the next token in the stream to have a particular string as its
// contents. If not, fail with an error on the token.
void expect_token_string(std::string str,
                         std::deque<std::shared_ptr<Token>> &tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
  }

  auto token = tokens.front();
  if (token->string_value != str) {
    err_token(token, "syntax error: expected '" + str + "', but got '" +
                         token->string_value + "' ");
  }

  tokens.pop_front();
}

void expect_any_token(std::deque<std::shared_ptr<Token>>& tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
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

  auto token = tokens.front();
  if (token->type != type) {
    // TODO: better errors
    err_token(token, "syntax error: unexpected token type; got " + token->string_value);
  }

  tokens.pop_front();
}

ExpNode_p parse_int_expr(std::deque<std::shared_ptr<Token>> &tokens) {
  assert(tokens.front()->type == TokenType::IntLiteral);
  int val = std::atoi(tokens.front()->string_value.c_str());
  expect_token_type(TokenType::IntLiteral, tokens);
  printf("Found integer %d\n", val);
  return new_int_node(val);
}

StmtNode_p parse_var(std::deque<std::shared_ptr<Token>> &tokens) {
  auto p = std::make_unique<StmtNode>();
  tokens.pop_front();
  return std::make_unique<StmtNode>();
}

StmtNode_p parse_return(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordReturn, tokens);
  ExpNode_p ret_exp = parse_expression(tokens);
  expect_token_type(TokenType::Semicolon, tokens);
  printf("returning ret_node\n");
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