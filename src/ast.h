#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <deque>

#include "token.h"

typedef enum {
  Invalid,
  // Infix operators
  OpOr,
  OpAnd,
  OpBor,
  OpXor,
  OpBand,
  OpNe,
  OpEq,
  OpGt,
  OpGe,
  OpLt,
  OpLe,
  OpPlus,
  OpMinus,
  OpTimes,
  OpDiv,
  OpRem,

  // Prefix operators
  OpNot,
  OpNeg,

  // Postfix operators
  OpSub
} Operator;

typedef enum { IntType, StringType, VoidType } Type;

typedef struct ExpNode {

  // The first three are terminals, the last two are nonterminals. 
  enum { IntExp, StringExp, VarExp, BinopExp, UnopExp, CallExp } kind;

  typedef struct {
    Operator op;
    std::shared_ptr<ExpNode> e;
  } UnOps;

  typedef struct {
    std::shared_ptr<ExpNode> e1;
    Operator op;
    std::shared_ptr<ExpNode> e2;
  } BinOps;

  typedef struct {
    std::string name;
    std::vector<std::shared_ptr<ExpNode>> args;
  } CallOps;

  typedef struct {
    std::string name;
  } VarOps;

  // TODO: Add IntrinsicOps

  std::variant<int, std::string, BinOps, UnOps, CallOps, VarOps> data;

  std::string to_str() {
    std::string op_str = "";
    switch (kind) {
      case IntExp: return "(" + std::to_string(std::get<int>(data)) + ")";
      case BinopExp: {
        BinOps ops = std::get<BinOps>(data);
        switch (ops.op) {
          case Operator::OpPlus: {op_str = "+"; break; }
          case Operator::OpTimes: {op_str = "*"; break; }
          case Operator::OpDiv: {op_str = "/"; break;}
          default: { 
            printf("to_str(): found unrecognized binary operator\n"); 
            exit(-1); 
          }
        }
        return "(" + ops.e1->to_str() + op_str + ops.e2->to_str() + ")";
      }
      case UnopExp: {
        UnOps ops = std::get<UnOps>(data);
        switch (ops.op) {
          case Operator::OpMinus: {op_str = "-"; break; }
          case Operator::OpNot: { op_str = "!"; break; }
          default: { 
            printf("to_str(): found unrecognized unary operator\n"); 
            exit(-1); 
          }
        }
        return "(" + op_str + " " + ops.e->to_str() + ")";
      }
      default: {
        printf("ERROR in to_str(): unhandled case\n");
        exit(-1);
      };
    }
  }

} ExpNode;

typedef struct StmtNode {
  enum { AssignStmt, IfStmt, WhileStmt, RepeatStmt, CallStmt, RetStmt } kind;

  typedef struct {
    std::string lhs;
    std::shared_ptr<ExpNode> rhs;
  } AssignOps;

  typedef struct {
    std::shared_ptr<ExpNode> cond;
    std::vector<std::shared_ptr<StmtNode>> then_stmts;
    std::vector<std::shared_ptr<StmtNode>> else_stmts;
  } IfOps;

  typedef struct {
    std::shared_ptr<ExpNode> cond;
    std::vector<std::shared_ptr<StmtNode>> body_stmts;
    std::vector<std::shared_ptr<StmtNode>> otherwise_stmts;
  } WhileOps;

  typedef struct {
    std::shared_ptr<ExpNode> cond;
    std::vector<std::shared_ptr<StmtNode>> body_stmts;
  } RepeatOps;

  typedef struct {
    std::string name;
    std::vector<std::shared_ptr<ExpNode>> args;
  } CallOps;

  typedef struct {
    std::shared_ptr<ExpNode> ret_exp;
  } RetOps;

  // TODO: Add IntrinsicStmt

  std::variant<AssignOps, IfOps, WhileOps, RepeatOps, CallOps, RetOps> data;
} StmtNode;

using StmtNode_p = std::shared_ptr<StmtNode>;
using ExpNode_p = std::shared_ptr<ExpNode>;
using StmtOps =
    std::variant<StmtNode::AssignOps, StmtNode::IfOps, StmtNode::WhileOps,
                 StmtNode::RepeatOps, StmtNode::CallOps, StmtNode::RetOps>;
using ExpOps = std::variant<int, std::string, ExpNode::BinOps, ExpNode::UnOps,
                            ExpNode::CallOps, ExpNode::VarOps>;

ExpNode_p exp_bp(std::deque<std::shared_ptr<Token>>& tokens, int bp);
ExpNode_p parse_expression(std::deque<std::shared_ptr<Token>>& tokens);
ExpNode_p new_int_node(int ival);
ExpNode_p new_unop_node(Operator op, ExpNode_p e);
ExpNode_p new_binop_node(Operator op, ExpNode_p lhs, ExpNode_p rhs);

StmtNode_p new_assign_node(std::string lhs, ExpNode_p rhs);
StmtNode_p new_if_node(ExpNode_p cond, std::vector<StmtNode_p> &then_stmts,
                       std::vector<StmtNode_p> &else_stmts);
StmtNode_p new_while_node(ExpNode_p cond, std::vector<StmtNode_p> &body_stmts,
                       std::vector<StmtNode_p> &otherwise_stmts);
StmtNode_p new_return_node(ExpNode_p ret_exp);
StmtNode_p new_repeat_node(ExpNode_p cond, std::vector<StmtNode_p>& body_stmts);