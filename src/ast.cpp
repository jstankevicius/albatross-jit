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
