#include "symres.h"
#include "ast.h"
#include "error.h"

void resolve_exp(ExpNode_p exp, SymbolTable &st) {
  switch (exp->kind) {
  case ExpNode::IntExp:
    return;
  case ExpNode::StringExp:
    return;
  case ExpNode::VarExp: {
    auto &ops = exp->var_ops();
    auto res = st.find_symbol(ops.name);

    // If res has no value, there was no corresponding vardecl that declared
    // this variable. Error out:
    if (!res.has_value()) {
      // TODO: Better errors:
      throw AlbatrossError("Could not find symbol " + ops.name, exp->line_num,
                           exp->col_num, EXIT_SYMRES_FAILURE);
    }

    // Otherwise, update this var node with the type and index.
    ops.var_info = res;
    return;
  }
  case ExpNode::BinopExp: {
    auto &ops = exp->bin_ops();
    resolve_exp(ops.e1, st);
    resolve_exp(ops.e2, st);
    return;
  }
  case ExpNode::UnopExp: {
    auto &ops = exp->un_ops();
    resolve_exp(ops.e, st);
    return;
  }
  case ExpNode::CallExp: {
    auto &ops = exp->call_ops();
    auto res = st.find_function(ops.name);

    if (!res.has_value()) {
      throw AlbatrossError("Undefined function " + ops.name, exp->line_num,
                           exp->col_num, EXIT_SYMRES_FAILURE);
    }

    // If the function exists, resolve its args:
    for (auto arg : ops.args) {
      resolve_exp(arg, st);
    }
    ops.fun_info = res;

    return;
  }
  }
}

inline void resolve_stmt(StmtNode_p stmt, SymbolTable &st) {
  switch (stmt->kind) {
  case StmtNode::VardeclStmt: {
    auto &ops = stmt->vardecl_ops();
    Type type = ops.type;
    auto &name = ops.lhs;

    // Check that we are not redeclaring the variable.
    if (st.cur_scope()->find_symbol(name)) {
      throw AlbatrossError("Redefinition of variable " + name, stmt->line_num,
                           stmt->col_num, EXIT_SYMRES_FAILURE);
    }

    resolve_exp(ops.rhs, st);

    // Construct a VarInfo struct for this variable.
    st.add_symbol(name, type);
    return;
  }

  case StmtNode::AssignStmt: {
    auto &ops = stmt->assign_ops();
    resolve_exp(ops.lhs, st);
    resolve_exp(ops.rhs, st);
    return;
  }

  case StmtNode::IfStmt: {
    auto &ops = stmt->if_ops();

    st.enter_scope();
    resolve_stmts(ops.then_stmts, st);
    st.exit_scope();
    return;
  }

  case StmtNode::WhileStmt: {
    auto &ops = stmt->while_ops();

    resolve_exp(ops.cond, st);
    st.enter_scope();
    resolve_stmts(ops.body_stmts, st);
    st.exit_scope();

    st.enter_scope();
    resolve_stmts(ops.otherwise_stmts, st);
    st.exit_scope();
    return;
  }

  case StmtNode::RepeatStmt: {
    auto &ops = stmt->repeat_ops();
    resolve_exp(ops.cond, st);

    st.enter_scope();
    resolve_stmts(ops.body_stmts, st);
    st.exit_scope();
    return;
  }

  case StmtNode::CallStmt: {
    auto &ops = stmt->call_ops();

    auto res = st.find_function(ops.name);

    if (!res.has_value()) {
      throw AlbatrossError("Undefined function " + ops.name, stmt->line_num,
                           stmt->col_num, EXIT_SYMRES_FAILURE);
    }

    for (auto arg : ops.args) {
      resolve_exp(arg, st);
    }
    ops.fun_info = res;
    return;
  }

  case StmtNode::FundecStmt: {
    auto &ops = stmt->fundec_ops();
    // Make sure we're not redeclaring a function.
    if (st.cur_scope()->find_function(ops.name)) {
      throw AlbatrossError("Redefinition of function " + ops.name,
                           stmt->line_num, stmt->col_num, EXIT_SYMRES_FAILURE);
    }

    st.add_function(ops.name, ops.ret_type, ops.params);
    st.enter_scope();

    // Add parameters into the scope of the function body.
    for (auto p : ops.params) {
      st.add_symbol(p.name, p.type);
    }

    resolve_stmts(ops.body, st);
    st.exit_scope();

    return;
  }

  case StmtNode::RetStmt: {
    auto ops = stmt->ret_ops();
    if (ops.ret_exp.get() != nullptr) {
      resolve_exp(stmt->ret_ops().ret_exp, st);
    }
    return;
  }
  }
}

void resolve_stmts(std::vector<StmtNode_p> &stmts, SymbolTable &st) {
  for (auto stmt : stmts) {
    resolve_stmt(stmt, st);
  }
}