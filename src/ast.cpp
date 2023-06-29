#include <memory>

#include "ast.h"
#include "token.h"

template <class T> inline auto assign_to(T &dst) {
  return [&dst](auto const &src) { dst = src; };
}

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
