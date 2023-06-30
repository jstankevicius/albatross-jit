#include <memory>

#include "ast.h"
#include "token.h"

template <class T> inline auto assign_to(T &dst) {
  return [&dst](auto const &src) { dst = src; };
}

// TODO: These should all be simplified, since there's a TON of repeated code.
ExpNode_p new_int_node(int ival) {
  auto p = std::make_shared<ExpNode>();
  ExpOps i = ival;
  std::visit(assign_to(p->data), i);
  return p;
}

ExpNode_p new_binop_node(Operator op, ExpNode_p lhs, ExpNode_p rhs) {
  auto p = std::make_shared<ExpNode>();
  ExpOps binops = ExpNode::BinOps{lhs, op, rhs}; 
  p->kind = ExpNode::BinopExp;
  std::visit(assign_to(p->data), binops);
  return p;
}

ExpNode_p new_unop_node(Operator op, ExpNode_p e) {
  auto p = std::make_shared<ExpNode>();
  ExpOps unops = ExpNode::UnOps{op, e};
  p->kind = ExpNode::UnopExp;
  std::visit(assign_to(p->data), unops);
  return p;
}


StmtNode_p new_assign_node(std::string lhs, ExpNode_p rhs) {
  auto p = std::make_shared<StmtNode>();
  StmtOps assign = StmtNode::AssignOps{lhs, rhs};
  std::visit(assign_to(p->data), assign);
  return p;
}

StmtNode_p new_return_node(ExpNode_p ret_exp) {
  auto p = std::make_shared<StmtNode>();
  StmtOps ret = StmtNode::RetOps{ret_exp};
  std::visit(assign_to(p->data), ret);
  return p;
}

StmtNode_p new_if_node(ExpNode_p cond, std::vector<StmtNode_p>& then_stmts, std::vector<StmtNode_p>& else_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps if_stmt = StmtNode::IfOps{cond, then_stmts, else_stmts};
  std::visit(assign_to(p->data), if_stmt);
  return p;
}

StmtNode_p new_while_node(ExpNode_p cond, std::vector<StmtNode_p>& body_stmts, std::vector<StmtNode_p>& otherwise_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps while_stmt = StmtNode::WhileOps{cond, body_stmts, otherwise_stmts};
  std::visit(assign_to(p->data), while_stmt);
  return p;
}

StmtNode_p new_repeat_node(ExpNode_p cond, std::vector<StmtNode_p>& body_stmts) {
  auto p = std::make_shared<StmtNode>();
  StmtOps repeat_stmt = StmtNode::RepeatOps{cond, body_stmts};
  std::visit(assign_to(p->data), repeat_stmt);
  return p;
}