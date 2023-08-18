#include "typecheck.h"
#include "compiler_stages.h"
#include "error.h"

#include <iostream>

Type
typecheck_exp(std::shared_ptr<ExpNode> exp)
{
        // TODO: This is hacky.
        if (exp.get() == nullptr) {
                return VoidType;
        }

        switch (exp->kind) {
        case ExpNode::IntExp: {
                return IntType;
        }
        case ExpNode::StringExp: {
                return StringType;
        }
        case ExpNode::VarExp: {
                auto &ops  = exp->var_ops();
                Type  type = ops.var_info.value().var_type;
#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Variable read \"" << ops.name << "\" type "
                          << type_to_str(type) << "\n";
#endif
#endif
#endif
#endif
                return type;
        }
        case ExpNode::BinopExp: {
                auto &ops      = exp->bin_ops();
                Type  type_lhs = typecheck_exp(ops.lhs);
                Type  type_rhs = typecheck_exp(ops.rhs);

                // TODO: Stricter typechecks here
                if (type_lhs != type_rhs) {
                        throw AlbatrossError("Mismatched types in binop",
                                             exp->line_num,
                                             exp->col_num,
                                             EXIT_TYPECHECK_FAILURE);
                }
                return type_lhs;
        }
        case ExpNode::UnopExp: {
                auto &ops  = exp->un_ops();
                Type  type = typecheck_exp(ops.e);
                return type;
        }
        case ExpNode::CallExp: {
                auto   &ops  = exp->call_ops();
                FunInfo info = ops.fun_info.value();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function called \"" << ops.name << "\" returns "
                          << type_to_str(info.ret_type) << "\n";
#endif
#endif
#endif
#endif

                // Check that the argument types match the parameter types.
                int n_params = info.params.size();
                int n_args   = ops.args.size();
                if (n_args != n_params) {
                        throw AlbatrossError(
                                "Incorrect number of arguments supplied for function "
                                        + ops.name + ": expected "
                                        + std::to_string(n_params) + ", got "
                                        + std::to_string(n_args),
                                exp->line_num,
                                exp->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                for (int i = 0; i < n_params; i++) {
                        Type arg_type   = typecheck_exp(ops.args[i]);
                        Type param_type = info.params[i].type;

                        if (arg_type != param_type) {
                                throw AlbatrossError(
                                        "Mismatched type in function "
                                                + ops.name + " for param "
                                                + info.params[i].name
                                                + ", position "
                                                + std::to_string(i),
                                        exp->line_num,
                                        exp->col_num,
                                        EXIT_TYPECHECK_FAILURE);
                        }
                }
                return ops.fun_info.value().ret_type;
        }
        }
}

void
typecheck_stmt(std::shared_ptr<StmtNode> stmt,
               std::optional<Type>       fun_ret_type = std::nullopt)
{
        switch (stmt->kind) {
        case StmtNode::AssignStmt: {
                auto &ops = stmt->assign_ops();

                // TODO: lhs should be legal lval; it cannot be something like 2 + 3.
                // Although if it is a variable, this will work just fine.
                Type type_lhs = typecheck_exp(ops.lhs);
                Type type_rhs = typecheck_exp(ops.rhs);

                if (type_lhs != type_rhs) {
                        throw AlbatrossError("Mismatched types in assignment",
                                             stmt->line_num,
                                             stmt->col_num,
                                             EXIT_TYPECHECK_FAILURE);
                }
                return;
        }
        case StmtNode::VardeclStmt: {
                auto &ops      = stmt->vardecl_ops();
                Type  type_lhs = ops.type;
                Type  type_rhs = typecheck_exp(ops.rhs);
                if (type_lhs != type_rhs) {
                        throw AlbatrossError(
                                "Mismatched types in variable declaration",
                                stmt->line_num,
                                stmt->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Variable declared \"" << ops.lhs << "\" type "
                          << type_to_str(type_lhs) << "\n";
#endif
#endif
#endif
#endif
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
                auto   &ops  = stmt->call_ops();
                FunInfo info = ops.fun_info.value();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function called \"" << ops.name << "\" returns "
                          << type_to_str(info.ret_type) << "\n";
#endif
#endif
#endif
#endif

                // Check that the argument types match the parameter types.
                int n_params = info.params.size();
                int n_args   = ops.args.size();

                if (n_args != n_params) {
                        throw AlbatrossError(
                                "Incorrect number of arguments supplied for function "
                                        + ops.name + ": expected "
                                        + std::to_string(n_params) + ", got "
                                        + std::to_string(n_args),
                                stmt->line_num,
                                stmt->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                for (int i = 0; i < n_params; i++) {
                        Type arg_type   = typecheck_exp(ops.args[i]);
                        Type param_type = info.params[i].type;

                        if (arg_type != param_type) {
                                throw AlbatrossError(
                                        "Mismatched type in function "
                                                + ops.name + " for param "
                                                + info.params[i].name
                                                + ", position "
                                                + std::to_string(i),
                                        stmt->line_num,
                                        stmt->col_num,
                                        EXIT_TYPECHECK_FAILURE);
                        }
                }
                for (auto exp : ops.args) {
                        typecheck_exp(exp);
                }

                return;
        }
        case StmtNode::RetStmt: {
                Type ret_exp_type =
                        stmt->ret_ops().ret_exp.get() != nullptr ?
                                typecheck_exp(stmt->ret_ops().ret_exp) :
                                VoidType;

                if (fun_ret_type && fun_ret_type.value() != ret_exp_type) {
                        throw AlbatrossError(
                                "Return statement does not return type specified in "
                                "function declaration.",
                                stmt->line_num,
                                stmt->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                // If the above check fails, we are in the global scope and can return only
                // integers.
                if (!fun_ret_type && ret_exp_type != IntType) {
                        throw AlbatrossError(
                                "Return expression in global scope must be of type "
                                "'int', but got '"
                                        + type_to_str(ret_exp_type) + "\'",
                                stmt->line_num,
                                stmt->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                return;
        }
        case StmtNode::FundecStmt: {
                auto &ops = stmt->fundec_ops();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function declared \"" << ops.name << "\" returns "
                          << type_to_str(ops.ret_type) << "\n";
#endif
#endif
#endif
#endif
                typecheck_stmts(ops.body, ops.ret_type);
                return;
        }
        }
}

void
typecheck_stmts(std::vector<std::shared_ptr<StmtNode>> &stmts,
                std::optional<Type>                     fun_ret_type)
{
        for (auto stmt : stmts) {
                typecheck_stmt(stmt, fun_ret_type);
        }
}