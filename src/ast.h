#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "token.h"

typedef enum {
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
} BinOp;

typedef enum { OpNot, OpNeg } UnOp;

typedef enum { IntType, StringType, VoidType } Type;

typedef struct ExpNode {

  enum { IntExp, StringExp, BinOpExp, UnOpExp, CallExp, VarExp } kind;

  typedef struct {
    std::shared_ptr<ExpNode> e1;
    BinOp op;
    std::shared_ptr<ExpNode> e2;
  } BinOps;

  typedef struct {
    UnOp op;
    std::shared_ptr<ExpNode> e;
  } UnOps;

  typedef struct {
    std::string name;
    std::vector<std::shared_ptr<ExpNode>> args;
  } CallOps;

  typedef struct {
    std::string name;
  } VarOps;

  // TODO: Add IntrinsicOps

  std::variant<int, std::string, BinOps, UnOps, CallOps, VarOps> data;

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

ExpNode_p new_int_node(int ival);

StmtNode_p new_assign_node(std::string lhs, ExpNode_p rhs);
StmtNode_p new_if_node(ExpNode_p cond, std::vector<StmtNode_p> &then_stmts,
                       std::vector<StmtNode_p> &else_stmts);

StmtNode_p new_return_node(ExpNode_p ret_exp);