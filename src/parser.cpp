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

Type str_to_type(std::string type_str) {
  if (type_str == "int")
    return Type::IntType;
  else if (type_str == "string")
    return Type::StringType;
  else if (type_str == "void")
    return Type::VoidType;
  else {
    printf("Invalid type %s\n", type_str.c_str());
    exit(-1);
  }
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

std::shared_ptr<Token>
expect_any_token(std::deque<std::shared_ptr<Token>> &tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
  }
  auto front = tokens.front();
  tokens.pop_front();
  return front;
}

// Expect the next token in the stream to have a particular type. If not, fail
// with an error on the token.
std::shared_ptr<Token>
expect_token_type(TokenType type, std::deque<std::shared_ptr<Token>> &tokens) {
  if (tokens.size() == 0) {
    printf("Unexpected EOF at end of file\n");
    exit(-1);
  }

  auto token = tokens.front();
  if (token->type != type) {
    // TODO: better errors
    err_token(token, "syntax error: unexpected token type; got " +
                         token->string_value);
  }

  tokens.pop_front();
  return token;
}

struct OpInfo {
  Operator op;

  int l_bp;
  int r_bp;

  enum { Prefix, Infix, Postfix, Invalid } kind;
};

// Return an operator's left and right binding power.
// TODO: This doesn't need to take the entire token queue.
OpInfo op_binding_power(const std::deque<std::shared_ptr<Token>> &tokens,
                        bool minus_prefix_flag = false) {
  TokenType op = tokens.front()->type;

  // In decreasing order of precedence:
  switch (op) {
  case TokenType::Lbracket:
    return OpInfo{Operator::OpSub, 200, -1, OpInfo::Postfix};
  case TokenType::OpNot:
    return OpInfo{Operator::OpNot, -1, 190, OpInfo::Prefix};
  case TokenType::OpTimes:
    return OpInfo{Operator::OpTimes, 175, 180, OpInfo::Infix};
  case TokenType::OpDiv:
    return OpInfo{Operator::OpDiv, 175, 180, OpInfo::Infix};
  case TokenType::OpRem:
    return OpInfo{Operator::OpRem, 175, 180, OpInfo::Infix};
  case TokenType::OpPlus:
    return OpInfo{Operator::OpPlus, 165, 170, OpInfo::Infix};
  case TokenType::OpMinus:
    return minus_prefix_flag
               ? OpInfo{Operator::OpMinus, -1, 190, OpInfo::Prefix}
               : OpInfo{Operator::OpMinus, 165, 170, OpInfo::Infix};
  case TokenType::OpLt:
    return OpInfo{Operator::OpLt, 145, 150, OpInfo::Infix};
  case TokenType::OpLe:
    return OpInfo{Operator::OpLe, 145, 150, OpInfo::Infix};
  case TokenType::OpGt:
    return OpInfo{Operator::OpGt, 145, 150, OpInfo::Infix};
  case TokenType::OpGe:
    return OpInfo{Operator::OpGe, 145, 150, OpInfo::Infix};
  case TokenType::OpEq:
    return OpInfo{Operator::OpEq, 135, 140, OpInfo::Infix};
  case TokenType::OpNe:
    return OpInfo{Operator::OpNe, 135, 140, OpInfo::Infix};
  case TokenType::OpBand:
    return OpInfo{Operator::OpBand, 125, 130, OpInfo::Infix};
  case TokenType::OpXor:
    return OpInfo{Operator::OpXor, 115, 120, OpInfo::Infix};
  case TokenType::OpBor:
    return OpInfo{Operator::OpBor, 105, 110, OpInfo::Infix};
  case TokenType::OpAnd:
    return OpInfo{Operator::OpAnd, 95, 100, OpInfo::Infix};
  case TokenType::OpOr:
    return OpInfo{Operator::OpOr, 85, 90, OpInfo::Infix};
  default:
    return OpInfo{Operator::Invalid, -1, -1, OpInfo::Invalid};
  }
}

ExpNode_p parse_var(std::deque<std::shared_ptr<Token>> &tokens) {
  auto name = expect_token_type(TokenType::Identifier, tokens)->string_value;
  return new_var_node(name);
}

ExpNode_p parse_str(std::deque<std::shared_ptr<Token>> &tokens) {
  auto str = expect_token_type(TokenType::StrLiteral, tokens)->string_value;
  return new_str_node(str);
}

ExpNode_p parse_int_expr(std::deque<std::shared_ptr<Token>> &tokens) {
  assert(tokens.front()->type == TokenType::IntLiteral);
  int val = std::atoi(tokens.front()->string_value.c_str());
  expect_token_type(TokenType::IntLiteral, tokens);
  return new_int_node(val);
}

ExpNode_p parse_call_exp(std::deque<std::shared_ptr<Token>> &tokens) {
  auto name = expect_token_type(TokenType::Identifier, tokens)->string_value;
  expect_token_type(TokenType::Lparen, tokens);
  std::vector<ExpNode_p> args;
  if (tokens.front()->type != TokenType::Rparen) {
    while (1) {
      auto arg = parse_expression(tokens);
      args.push_back(arg);
      if (tokens.front()->type == TokenType::Comma) {
        expect_token_type(TokenType::Comma, tokens);
        continue;
      } else {
        expect_token_type(TokenType::Rparen, tokens);
        break;
      }
    }
  } else {
    expect_token_type(TokenType::Rparen, tokens);
  }

  return new_call_expr(name, args);
}

// Pratt's parse() function. Recursively builds an expression AST from a token
// stream.
ExpNode_p exp_bp(std::deque<std::shared_ptr<Token>> &tokens, int min_bp) {

  auto front = tokens.front();
  ExpNode_p lhs;
  switch (front->type) {
  case TokenType::IntLiteral: {
    lhs = parse_int_expr(tokens);
    break;
  };
  case TokenType::Identifier: {
    // Check if this is a function call or just an identifier:
    if (tokens.at(1)->type == TokenType::Lparen) {
      lhs = parse_call_exp(tokens);
    } else {
      lhs = parse_var(tokens);
    }

    break;
  }

  case TokenType::StrLiteral: {
    lhs = parse_str(tokens);
    break;
  }

  // TODO: Add identifiers
  case TokenType::Lparen: {
    expect_token_type(TokenType::Lparen, tokens);
    lhs = exp_bp(tokens, 0);
    expect_token_type(TokenType::Rparen, tokens);
    break;
  }

  // Well, it can pretty much ONLY be a prefix operator.
  case TokenType::OpMinus:
  case TokenType::OpNot: {
    OpInfo info = op_binding_power(tokens, true);
    assert(info.kind == OpInfo::Prefix);
    auto r_bp = info.r_bp;

    // Consume the operator; it is guaranteed to be either OpMinus or OpNot
    expect_any_token(tokens);
    auto rhs = exp_bp(tokens, r_bp);

    lhs = new_unop_node(info.op, rhs);
    break;
  }

  default:
    err_token(front, "Expected literal, identifier, paren, or prefix operator");
  }

  while (1) {

    // Check for EOF:
    if (tokens.front()->type == TokenType::Eof) {
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

    // Anything that isn't an infix or postfix token (like a paren or semicolon)
    // will fall through to this `break` and exit the loop. This will hand
    // control back to whatever function called parse_expression so that we can
    break;
  }

  return lhs;
}

// Parse an expression from the token stream.
ExpNode_p parse_expression(std::deque<std::shared_ptr<Token>> &tokens) {
  auto e = exp_bp(tokens, 0);
  // std::cout << e->to_str() << "\n";
  return e;
}

StmtNode_p parse_vardecl(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordVar, tokens);
  auto name = expect_token_type(TokenType::Identifier, tokens)->string_value;
  auto type =
      str_to_type(expect_token_type(TokenType::TypeName, tokens)->string_value);
  expect_token_type(TokenType::Assign, tokens);
  ExpNode_p rhs = parse_expression(tokens);
  expect_token_type(TokenType::Semicolon, tokens);

  return new_vardec_node(name, type, rhs);
}

StmtNode_p parse_assign(std::deque<std::shared_ptr<Token>> &tokens) {
  ExpNode_p lhs = parse_expression(tokens);
  expect_token_type(TokenType::Assign, tokens);
  ExpNode_p rhs = parse_expression(tokens);
  expect_token_type(TokenType::Semicolon, tokens);
  return new_assign_node(lhs, rhs);
}

StmtNode_p parse_return(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordReturn, tokens);
  ExpNode_p ret_exp = parse_expression(tokens);
  expect_token_type(TokenType::Semicolon, tokens);
  return new_return_node(ret_exp);
}

StmtNode_p parse_if(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordIf, tokens);
  // Should parentheses be optional?
  ExpNode_p cond = parse_expression(tokens);

  std::vector<StmtNode_p> then_stmts;
  expect_token_type(TokenType::Lcurl, tokens);
  while (tokens.front()->type != TokenType::Rcurl) {
    then_stmts.push_back(parse_stmt(tokens));
  }
  expect_token_type(TokenType::Rcurl, tokens);
  std::vector<StmtNode_p> else_stmts;

  // TODO: Add an EOF token so we don't have to do this kind of dumb checking.
  if (!tokens.empty() && tokens.front()->type == TokenType::KeywordElse) {
    expect_token_type(TokenType::KeywordElse, tokens);
    expect_token_type(TokenType::Lcurl, tokens);
    while (tokens.front()->type != TokenType::Rcurl) {
      else_stmts.push_back(parse_stmt(tokens));
    }
    expect_token_type(TokenType::Rcurl, tokens);
  }
  return new_if_node(cond, then_stmts, else_stmts);
}

StmtNode_p parse_while(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordWhile, tokens);
  ExpNode_p cond = parse_expression(tokens);

  std::vector<StmtNode_p> body_stmts;
  expect_token_type(TokenType::Lcurl, tokens);
  while (tokens.front()->type != TokenType::Rcurl) {
    body_stmts.push_back(parse_stmt(tokens));
  }
  expect_token_type(TokenType::Rcurl, tokens);

  std::vector<StmtNode_p> otherwise_stmts;
  if (!tokens.empty() && tokens.front()->type == TokenType::KeywordOtherwise) {
    expect_token_type(TokenType::KeywordOtherwise, tokens);
    expect_token_type(TokenType::Lcurl, tokens);
    while (tokens.front()->type != TokenType::Rcurl) {
      otherwise_stmts.push_back(parse_stmt(tokens));
    }
    expect_token_type(TokenType::Rcurl, tokens);
  }

  return new_while_node(cond, body_stmts, otherwise_stmts);
}

StmtNode_p parse_repeat(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordRepeat, tokens);
  ExpNode_p cond = parse_expression(tokens);
  expect_token_type(TokenType::Lcurl, tokens);

  std::vector<StmtNode_p> body_stmts;
  while (tokens.front()->type != TokenType::Rcurl) {
    body_stmts.push_back(parse_stmt(tokens));
  }

  expect_token_type(TokenType::Rcurl, tokens);
  return new_repeat_node(cond, body_stmts);
}

StmtNode_p parse_fundecl(std::deque<std::shared_ptr<Token>> &tokens) {
  expect_token_type(TokenType::KeywordFun, tokens);

  auto fun_name =
      expect_token_type(TokenType::Identifier, tokens)->string_value;

  // TODO: We can (maybe) make type declarations optional for functions.
  // Instead, infer from the types of all return statements in the function.
  auto type =
      str_to_type(expect_token_type(TokenType::TypeName, tokens)->string_value);
  expect_token_type(TokenType::Lparen, tokens);

  // TODO: Should this be a pointer? Like TypeNode_p?
  std::vector<TypeNode> params;
  if (tokens.front()->type != TokenType::Rparen) {
    while (1) {
      // OK, we probably just want to assign a string value to the name instead
      // of going through a pointer deref whenever we want to find out the name
      // of the parameter.
      auto param_name = expect_token_type(TokenType::Identifier, tokens);
      auto param_type =
          expect_token_type(TokenType::TypeName, tokens)->string_value;

      params.push_back(TypeNode{param_name, str_to_type(param_type)});

      if (tokens.front()->type == TokenType::Comma) {
        expect_token_type(TokenType::Comma, tokens);
        continue;
      } else {
        expect_token_type(TokenType::Rparen, tokens);
        break;
      }
    }
  } else {
    expect_token_type(TokenType::Rparen, tokens);
  }

  expect_token_type(TokenType::Lcurl, tokens);

  std::vector<StmtNode_p> body_stmts;
  while (tokens.front()->type != TokenType::Rcurl) {
    body_stmts.push_back(parse_stmt(tokens));
  }
  expect_token_type(TokenType::Rcurl, tokens);

  printf("Done parsing function\n");
  return new_fundec_node(fun_name, type, params, body_stmts);
}


StmtNode_p parse_call_stmt(std::deque<std::shared_ptr<Token>> &tokens) {

  auto name = expect_token_type(TokenType::Identifier, tokens)->string_value;
  expect_token_type(TokenType::Lparen, tokens);

  std::vector<ExpNode_p> args;
  if (tokens.front()->type != TokenType::Rparen) {
    while (1) {
      auto arg = parse_expression(tokens);
      args.push_back(arg);
      if (tokens.front()->type == TokenType::Comma) {
        expect_token_type(TokenType::Comma, tokens);
        continue;
      } else {
        expect_token_type(TokenType::Rparen, tokens);
        break;
      }
    }
  } else {
    expect_token_type(TokenType::Rparen, tokens);
  }
  expect_token_type(TokenType::Semicolon, tokens);
  return new_call_node(name, args);
}

StmtNode_p parse_stmt(std::deque<std::shared_ptr<Token>> &tokens) {
  // Parse a top-level statement and return its AST.
  const auto front = tokens.front();
  auto p = std::make_shared<StmtNode>();

  switch (front->type) {
  case TokenType::Identifier: {
    if (tokens.at(1)->type != TokenType::Lparen) {
      return parse_assign(tokens);
    } else {
      return parse_call_stmt(tokens);
    }
  }
  case TokenType::KeywordVar:
    return parse_vardecl(tokens);
  case TokenType::KeywordReturn:
    return parse_return(tokens);
  case TokenType::KeywordIf:
    return parse_if(tokens);
  case TokenType::KeywordWhile:
    return parse_while(tokens);
  case TokenType::KeywordRepeat:
    return parse_repeat(tokens);
  case TokenType::KeywordFun:
    return parse_fundecl(tokens);
  default:
    err_token(front, "expected a statement");
  }

  return p;
}

std::vector<StmtNode_p>
parse_stmts(std::deque<std::shared_ptr<Token>> &tokens) {
  std::vector<StmtNode_p> stmts;
  while (tokens.front()->type != TokenType::Eof) {
    stmts.push_back(parse_stmt(tokens));
  }
  return stmts;
}