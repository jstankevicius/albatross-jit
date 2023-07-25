#pragma once

#include <deque>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "token.h"
#include "types.h"

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

typedef struct TypeNode {
  std::string name;
  Type type;
} TypeNode;

typedef struct VarInfo {
  Type var_type;

  // This variable's DeBruijn index. Denotes a generic "memory location"
  // where that variable may be stored. Initialized during symbol resolution.
  // Example:
  // var a int := 1; <-- DBI of 0
  // if (a < 2) {
  //   var b int := 123; <-- DBI of 1
  // }
  // var c int := 4; <-- DBI of 1; b has gone out of scope, so its memory
  //                     location may be reused.
  int var_idx_db;
} VarInfo;

typedef struct FunInfo {
  Type ret_type;
  int var_idx_db;

  std::vector<TypeNode> params;
} FunInfo;

typedef struct ExpNode {

  int line_num = -1;
  int col_num = -1;

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
    std::optional<FunInfo> fun_info;
  } CallOps;

  typedef struct {
    std::string name;
    std::optional<VarInfo> var_info;
  } VarOps;

  // TODO: Add IntrinsicOps
  std::variant<int, std::string, BinOps, UnOps, CallOps, VarOps> data;

  inline int int_ops() { return std::get<int>(data); }

  inline std::string &str_ops() { return std::get<std::string>(data); }

  inline BinOps &bin_ops() { return std::get<BinOps>(data); }

  inline UnOps &un_ops() { return std::get<UnOps>(data); }

  inline CallOps &call_ops() { return std::get<CallOps>(data); }

  inline VarOps &var_ops() { return std::get<VarOps>(data); }

} ExpNode;

typedef struct StmtNode {

  int line_num = -1;
  int col_num = -1;

  enum StmtKind {
    AssignStmt,
    VardeclStmt,
    IfStmt,
    WhileStmt,
    RepeatStmt,
    CallStmt,
    FundecStmt,
    RetStmt
  } kind;

  typedef struct {
    std::shared_ptr<ExpNode> lhs;
    std::shared_ptr<ExpNode> rhs;
  } AssignOps;

  typedef struct {
    std::string lhs;
    Type type;
    std::shared_ptr<ExpNode> rhs;
  } VardeclOps;

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
    std::optional<FunInfo> fun_info;
  } CallOps;

  typedef struct {
    std::string name;
    Type ret_type;
    std::vector<TypeNode> params;
    std::vector<std::shared_ptr<StmtNode>> body;
  } FundecOps;

  typedef struct {
    std::shared_ptr<ExpNode> ret_exp;
  } RetOps;

  // TODO: Add IntrinsicStmt

  std::variant<VardeclOps, AssignOps, IfOps, WhileOps, RepeatOps, CallOps,
               FundecOps, RetOps>
      data;

  inline AssignOps &assign_ops() { return std::get<AssignOps>(data); }
  inline VardeclOps &vardecl_ops() { return std::get<VardeclOps>(data); }
  inline IfOps &if_ops() { return std::get<IfOps>(data); }
  inline WhileOps &while_ops() { return std::get<WhileOps>(data); }
  inline RepeatOps &repeat_ops() { return std::get<RepeatOps>(data); }
  inline CallOps &call_ops() { return std::get<CallOps>(data); }
  inline FundecOps &fundec_ops() { return std::get<FundecOps>(data); }
  inline RetOps &ret_ops() { return std::get<RetOps>(data); }
} StmtNode;

using StmtNode_p = std::shared_ptr<StmtNode>;
using ExpNode_p = std::shared_ptr<ExpNode>;
using StmtOps =
    std::variant<StmtNode::VardeclOps, StmtNode::AssignOps, StmtNode::IfOps,
                 StmtNode::WhileOps, StmtNode::RepeatOps, StmtNode::CallOps,
                 StmtNode::FundecOps, StmtNode::RetOps>;
using ExpOps = std::variant<int, std::string, ExpNode::BinOps, ExpNode::UnOps,
                            ExpNode::CallOps, ExpNode::VarOps>;

ExpNode_p new_int_exp_node(int ival);
ExpNode_p new_unop_exp_node(Operator op, ExpNode_p e);
ExpNode_p new_binop_exp_node(Operator op, ExpNode_p lhs, ExpNode_p rhs);
ExpNode_p new_var_exp_node(std::string name);
ExpNode_p new_str_exp_node(std::string str);
ExpNode_p new_call_exp_node(std::string name, std::vector<ExpNode_p> &args);

StmtNode_p new_assign_stmt_node(ExpNode_p lhs, ExpNode_p rhs);
StmtNode_p new_if_stmt_node(ExpNode_p cond, std::vector<StmtNode_p> &then_stmts,
                            std::vector<StmtNode_p> &else_stmts);
StmtNode_p new_while_stmt_node(ExpNode_p cond,
                               std::vector<StmtNode_p> &body_stmts,
                               std::vector<StmtNode_p> &otherwise_stmts);
StmtNode_p new_return_stmt_node(ExpNode_p ret_exp);
StmtNode_p new_repeat_stmt_node(ExpNode_p cond,
                                std::vector<StmtNode_p> &body_stmts);
StmtNode_p new_fundec_stmt_node(std::string fun_name, Type ret_type,
                                std::vector<TypeNode> &params,
                                std::vector<StmtNode_p> body);
StmtNode_p new_vardecl_stmt_node(std::string name, Type type, ExpNode_p rhs);
StmtNode_p new_call_stmt_node(std::string name, std::vector<ExpNode_p> args);