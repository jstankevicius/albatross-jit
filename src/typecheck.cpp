#include "typecheck.h"
#include "compiler_stages.h"
#include "error.h"

#include <iostream>

Type
typecheck_exp(ExpNode *exp)
{
        switch (exp->kind) {
        case ExpNode::IntExp: {
                return IntType;
        }
        case ExpNode::StringExp: {
                return StringType;
        }
        case ExpNode::VarExp: {
                auto node = dynamic_cast<VarNode *>(exp);
                Type type = node->var_info.value().var_type;

                // TODO: Distinguish between reading from a variable and writing
                // to a variable.
#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Variable read \"" << node->name << "\" type "
                          << type_to_str(type) << "\n";
#endif
#endif
#endif
#endif
                return type;
        }
        case ExpNode::BinopExp: {
                auto node     = dynamic_cast<BinOpNode *>(exp);
                Type type_lhs = typecheck_exp(node->lhs.get());
                Type type_rhs = typecheck_exp(node->rhs.get());

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
                auto node = dynamic_cast<UnOpNode *>(exp);
                Type type = typecheck_exp(node->e.get());
                return type;
        }
        case ExpNode::CallExp: {
                auto     node = dynamic_cast<CallNode *>(exp);
                FunInfo &info = node->fun_info.value();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function called \"" << node->name << "\" returns "
                          << type_to_str(info.ret_type) << "\n";
#endif
#endif
#endif
#endif

                // Check that the argument types match the parameter types.
                int n_params = info.params.size();
                int n_args   = node->args.size();
                if (n_args != n_params) {
                        throw AlbatrossError(
                                "Incorrect number of arguments supplied for function "
                                        + node->name + ": expected "
                                        + std::to_string(n_params) + ", got "
                                        + std::to_string(n_args),
                                exp->line_num,
                                exp->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                for (int i = 0; i < n_params; i++) {
                        Type arg_type   = typecheck_exp(node->args[i].get());
                        Type param_type = info.params[i].type;

                        if (arg_type != param_type) {
                                throw AlbatrossError(
                                        "Mismatched type in function "
                                                + node->name + " for param "
                                                + info.params[i].name
                                                + ", position "
                                                + std::to_string(i),
                                        exp->line_num,
                                        exp->col_num,
                                        EXIT_TYPECHECK_FAILURE);
                        }
                }
                return node->fun_info.value().ret_type;
        }
        }
}

void
typecheck_stmt(StmtNode *stmt, std::optional<Type> fun_ret_type = std::nullopt)
{
        switch (stmt->kind) {
        case StmtNode::AssignStmt: {
                auto node = dynamic_cast<AssignNode *>(stmt);

                // TODO: lhs should be legal lval; it cannot be something like 2 + 3.
                // Although if it is a variable, this will work just fine.
                Type type_lhs = typecheck_exp(node->lhs.get());
                Type type_rhs = typecheck_exp(node->rhs.get());

                if (type_lhs != type_rhs) {
                        throw AlbatrossError("Mismatched types in assignment",
                                             stmt->line_num,
                                             stmt->col_num,
                                             EXIT_TYPECHECK_FAILURE);
                }
                return;
        }
        case StmtNode::VardeclStmt: {
                auto node     = dynamic_cast<VardeclNode *>(stmt);
                Type type_lhs = node->type;
                Type type_rhs = typecheck_exp(node->rhs.get());
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
                std::cout << "Variable declared \"" << node->lhs << "\" type "
                          << type_to_str(type_lhs) << "\n";
#endif
#endif
#endif
#endif
                return;
        }
        case StmtNode::IfStmt: {
                auto node      = dynamic_cast<IfNode *>(stmt);
                Type cond_type = typecheck_exp(node->cond.get());
                if (cond_type != IntType) {
                        throw AlbatrossError(
                                "Condition expressions in if statements must be of type int, but got "
                                        + type_to_str(cond_type),
                                node->line_num,
                                node->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }

                typecheck_stmts(node->then_stmts);
                typecheck_stmts(node->else_stmts);
                return;
        }
        case StmtNode::WhileStmt: {
                auto node = dynamic_cast<WhileNode *>(stmt);

                Type cond_type = typecheck_exp(node->cond.get());
                if (cond_type != IntType) {
                        throw AlbatrossError(
                                "Condition expressions in while statements must be of type int, but got "
                                        + type_to_str(cond_type),
                                node->line_num,
                                node->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }

                typecheck_exp(node->cond.get());
                typecheck_stmts(node->body_stmts);
                typecheck_stmts(node->otherwise_stmts);
                return;
        }
        case StmtNode::RepeatStmt: {
                auto node = dynamic_cast<RepeatNode *>(stmt);

                Type cond_type = typecheck_exp(node->cond.get());
                if (cond_type != IntType) {
                        throw AlbatrossError(
                                "Condition expressions in repeat statements must be of type int, but got "
                                        + type_to_str(cond_type),
                                node->line_num,
                                node->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }

                typecheck_exp(node->cond.get());
                typecheck_stmts(node->body_stmts);
                return;
        }
        case StmtNode::CallStmt: {
                auto     node = dynamic_cast<CallStmtNode *>(stmt);
                FunInfo &info = node->fun_info.value();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function called \"" << node->name << "\" returns "
                          << type_to_str(info.ret_type) << "\n";
#endif
#endif
#endif
#endif

                // Check that the argument types match the parameter types.
                int n_params = info.params.size();
                int n_args   = node->args.size();

                if (n_args != n_params) {
                        throw AlbatrossError(
                                "Incorrect number of arguments supplied for function "
                                        + node->name + ": expected "
                                        + std::to_string(n_params) + ", got "
                                        + std::to_string(n_args),
                                stmt->line_num,
                                stmt->col_num,
                                EXIT_TYPECHECK_FAILURE);
                }
                for (int i = 0; i < n_params; i++) {
                        Type arg_type   = typecheck_exp(node->args[i].get());
                        Type param_type = info.params[i].type;

                        if (arg_type != param_type) {
                                throw AlbatrossError(
                                        "Mismatched type in function "
                                                + node->name + " for param "
                                                + info.params[i].name
                                                + ", position "
                                                + std::to_string(i),
                                        stmt->line_num,
                                        stmt->col_num,
                                        EXIT_TYPECHECK_FAILURE);
                        }
                }
                for (auto &arg : node->args) {
                        typecheck_exp(arg.get());
                }

                return;
        }
        case StmtNode::RetStmt: {
                auto node = dynamic_cast<RetNode *>(stmt);
                Type ret_exp_type =
                        node->ret_exp.has_value() ?
                                typecheck_exp(node->ret_exp.value().get()) :
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
                auto node = dynamic_cast<FundecNode *>(stmt);

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
                std::cout << "Function declared \"" << node->name
                          << "\" returns " << type_to_str(node->ret_type)
                          << "\n";
#endif
#endif
#endif
#endif
                typecheck_stmts(node->body, node->ret_type);
                return;
        }
        }
}

void
typecheck_stmts(std::vector<std::unique_ptr<StmtNode>> &stmts,
                std::optional<Type>                     fun_ret_type)
{
        for (auto &stmt : stmts) {
                typecheck_stmt(stmt.get(), fun_ret_type);
        }
}