#include "typecheck.h"
#include <iostream>

Type typecheck_exp(ExpNode_p exp) {
  switch (exp->kind) {
  case ExpNode::IntExp: {
    return IntType;
  }
  case ExpNode::StringExp: {
    return StringType;
  }
  case ExpNode::VarExp: {
    return exp->var_ops().var_info.value().var_type;
  }
  case ExpNode::BinopExp: {
    auto &ops = exp->bin_ops();
    Type type_lhs = typecheck_exp(ops.e1);
    Type type_rhs = typecheck_exp(ops.e2);

    if (type_lhs != type_rhs) {
      printf("Mismatched types in binop\n");
      exit(-1);
    }
    return type_lhs;
  }
  case ExpNode::UnopExp: {
    auto &ops = exp->un_ops();
    Type type = typecheck_exp(ops.e);
    return type;
  }
  case ExpNode::CallExp: {
    auto &ops = exp->call_ops();
    FunInfo info = ops.fun_info.value();

    // Check that the argument types match the parameter types.
    int n_params = info.params.size();
    int n_args = ops.args.size();
    if (n_args != n_params) {
      printf("Incorrect number of arguments supplied for function %s: expected "
             "%d, got %d\n",
             ops.name.c_str(), n_params, n_args);
      exit(-1);
    }
    for (int i = 0; i < n_params; i++) {
      Type arg_type = typecheck_exp(ops.args[i]);
      Type param_type = info.params[i].type;

      if (arg_type != param_type) {
        printf("Mismatched type in function %s for param %s, position %d\n",
               ops.name.c_str(), info.params[i].name.c_str(), i);
      }
    }
    return ops.fun_info.value().ret_type;
  }
  }
}

void typecheck_stmt(StmtNode_p stmt,
                    std::optional<Type> fun_ret_type = std::nullopt) {
  switch (stmt->kind) {
  case StmtNode::AssignStmt: {
    auto &ops = stmt->assign_ops();

    // TODO: lhs should be legal lval; it cannot be something like 2 + 3.
    // Although if it is a variable, this will work just fine.
    Type type_lhs = typecheck_exp(ops.lhs);
    Type type_rhs = typecheck_exp(ops.rhs);

    if (type_lhs != type_rhs) {
      printf("Mismatched types in assignment\n");
      exit(-1);
    }
    return;
  }
  case StmtNode::VardeclStmt: {
    printf("typechecking vardecl\n");
    auto &ops = stmt->vardecl_ops();
    Type type_lhs = ops.type;
    Type type_rhs = typecheck_exp(ops.rhs);
    if (type_lhs != type_rhs) {
      printf("Mismatched types in vardecl\n");
      exit(-1);
    }
    return;
  }
  case StmtNode::IfStmt: {
    auto &ops = stmt->if_ops();
    typecheck_stmts(ops.then_stmts);
    typecheck_stmts(ops.else_stmts);
    return;
  }
  case StmtNode::WhileStmt: {
    auto &ops = stmt->while_ops();
    typecheck_exp(ops.cond);
    typecheck_stmts(ops.body_stmts);
    typecheck_stmts(ops.otherwise_stmts);
    return;
  }
  case StmtNode::RepeatStmt: {
    auto &ops = stmt->repeat_ops();
    typecheck_exp(ops.cond);
    typecheck_stmts(ops.body_stmts);
    return;
  }
  case StmtNode::CallStmt: {
    auto &ops = stmt->call_ops();
    for (auto exp : ops.args) {
      typecheck_exp(exp);
    }
    return;
  }
  case StmtNode::RetStmt: {
    Type ret_exp_type = typecheck_exp(stmt->ret_ops().ret_exp);

    if (fun_ret_type && fun_ret_type.value() != ret_exp_type) {
      printf("Return statement does not return type specified in function "
             "declaration.\n");
      exit(-1);
    }
    return;
  }
  case StmtNode::FundecStmt: {
    auto &ops = stmt->fundec_ops();

    // When we recurse into the function statements, we need to check that every
    // return statement's type actually matches the function's return type.
    typecheck_stmts(ops.body, ops.ret_type);
    return;
  }
  }
}

void typecheck_stmts(std::vector<StmtNode_p> &stmts,
                     std::optional<Type> fun_ret_type) {
  for (auto stmt : stmts) {
    printf("Typechecking stmt\n");
    typecheck_stmt(stmt, fun_ret_type);
  }
}