#include <memory>

#include "ast.h"
#include "token.h"

template <class T> inline auto assign_to(T &dst) {
  return [&dst](auto const &src) { dst = src; };
}

ExpNode_p new_var_exp_node(std::string name) {
  auto p = std::make_shared<ExpNode>();
  ExpOps var_exp = ExpNode::VarOps{name};
  std::visit(assign_to(p->data), var_exp);
  return p;
}

ExpNode_p new_str_exp_node(std::string str) {
  auto p = std::make_shared<ExpNode>();
  ExpOps str_exp = str;
  p->kind = ExpNode::StringExp;
  std::visit(assign_to(p->data), str_exp);
  return p;
}

// TODO: These should all be simplified, since there's a TON of repeated code.
ExpNode_p new_int_exp_node(int ival) {
  auto p = std::make_shared<ExpNode>();
  ExpOps i = ival;
  p->kind = ExpNode::IntExp;
  std::visit(assign_to(p->data), i);
  return p;
}

ExpNode_p new_binop_exp_node(Operator op, ExpNode_p lhs, ExpNode_p rhs) {
  auto p = std::make_shared<ExpNode>();
  ExpOps binops = ExpNode::BinOps{lhs, op, rhs};
  p->kind = ExpNode::BinopExp;
  std::visit(assign_to(p->data), binops);
  return p;
}

ExpNode_p new_unop_exp_node(Operator op, ExpNode_p e) {
  auto p = std::make_shared<ExpNode>();
  ExpOps unops = ExpNode::UnOps{op, e};
  p->kind = ExpNode::UnopExp;
  std::visit(assign_to(p->data), unops);
  return p;
}

ExpNode_p new_call_exp_node(std::string name, std::vector<ExpNode_p> &args) {
  auto p = std::make_shared<ExpNode>();
  ExpOps unops = ExpNode::CallOps{name, args};
  p->kind = ExpNode::CallExp;
  std::visit(assign_to(p->data), unops);
  return p;
}

StmtNode_p new_assign_stmt_node(ExpNode_p lhs, ExpNode_p rhs) {
  auto p = std::make_shared<StmtNode>();
  StmtOps ret = StmtNode::AssignOps{lhs, rhs};
  std::visit(assign_to(p->data), ret);
  return p;
}

StmtNode_p new_vardecl_stmt_node(std::string name, Type type, ExpNode_p rhs) {
  auto p = std::make_shared<StmtNode>();
  StmtOps ret = StmtNode::VardecOps{name, type, rhs};
  std::visit(assign_to(p->data), ret);
  return p;
}

StmtNode_p new_return_stmt_node(ExpNode_p ret_exp) {
  auto p = std::make_shared<StmtNode>();
  StmtOps ret = StmtNode::RetOps{ret_exp};
  std::visit(assign_to(p->data), ret);
  return p;
}

StmtNode_p new_if_stmt_node(ExpNode_p cond, std::vector<StmtNode_p> &then_stmts,
                            std::vector<StmtNode_p> &else_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps if_stmt = StmtNode::IfOps{cond, then_stmts, else_stmts};
  std::visit(assign_to(p->data), if_stmt);
  return p;
}

StmtNode_p new_while_stmt_node(ExpNode_p cond,
                               std::vector<StmtNode_p> &body_stmts,
                               std::vector<StmtNode_p> &otherwise_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps while_stmt = StmtNode::WhileOps{cond, body_stmts, otherwise_stmts};
  std::visit(assign_to(p->data), while_stmt);
  return p;
}

StmtNode_p new_repeat_stmt_node(ExpNode_p cond,
                                std::vector<StmtNode_p> &body_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps repeat_stmt = StmtNode::RepeatOps{cond, body_stmts};
  std::visit(assign_to(p->data), repeat_stmt);
  return p;
}

StmtNode_p new_fundec_stmt_node(std::string fun_name, Type ret_type,
                                std::vector<TypeNode> &params,
                                std::vector<StmtNode_p> body) {
  auto p = std::make_shared<StmtNode>();
  StmtOps repeat_stmt = StmtNode::FundecOps{fun_name, ret_type, params, body};
  std::visit(assign_to(p->data), repeat_stmt);
  return p;
}

StmtNode_p new_call_stmt_node(std::string name, std::vector<ExpNode_p> args) {
  auto p = std::make_shared<StmtNode>();
  StmtOps call_stmt = StmtNode::CallOps{name, args};
  std::visit(assign_to(p->data), call_stmt);
  return p;
}