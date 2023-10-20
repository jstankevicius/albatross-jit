#include "typecheck.h"
#include "compiler_stages.h"
#include "error.h"

#include <iostream>

Type
TypecheckVisitor::typecheck_exp(ExpNode* node) {

    node->accept(*this);
    try {
        return node->value_type.value();
    } catch (std::bad_optional_access &e) {
        throw AlbatrossError(
            "Tried typechecking expression, but visitor left no type",
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }
}

void
TypecheckVisitor::visit_int_node(IntNode *node)
{
    node->value_type = Type::Int;
}

void
TypecheckVisitor::visit_string_node(StrNode *node)
{
    node->value_type = Type::String;
}

void
TypecheckVisitor::visit_var_node(VarNode *node)
{
    Type type        = node->var_info.value().var_type;
    node->value_type = type;
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
}

void
TypecheckVisitor::visit_binop_node(BinOpNode *node)
{
    Type t_lhs = typecheck_exp(node->lhs.get());
    Type t_rhs = typecheck_exp(node->rhs.get());

    if (t_lhs == Type::Int && t_rhs == Type::Int) {
        node->value_type = Type::Int;
    } else {
        throw AlbatrossError("Unsupported operands: " + type_to_str(t_lhs) + " "
                                 + op_str(node->op) + " " + type_to_str(t_rhs),
                             node->line_num,
                             node->col_num,
                             EXIT_TYPECHECK_FAILURE);
    }
}

void
TypecheckVisitor::visit_unop_node(UnOpNode *node)
{
    Type t = typecheck_exp(node->e.get());

    if (t == Type::Int) {
        node->value_type = Type::Int;
    } else {
        throw AlbatrossError("Unsupported operand: " + op_str(node->op) + " "
                                 + type_to_str(t),
                             node->line_num,
                             node->col_num,
                             EXIT_TYPECHECK_FAILURE);
    }
}

void
TypecheckVisitor::visit_call_node(CallNode *node)
{
    FunInfo &info    = node->fun_info.value();
    node->value_type = info.ret_type;
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
            "Incorrect number of arguments supplied for function " + node->name
                + ": expected " + std::to_string(n_params) + ", got "
                + std::to_string(n_args),
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    for (int i = 0; i < n_params; i++) {
        Type arg_type   = typecheck_exp(node->args[i].get());
        Type param_type = info.params[i].type;

        if (arg_type != param_type) {
            throw AlbatrossError("Mismatched type in function " + node->name
                                     + " for param " + info.params[i].name
                                     + ", position " + std::to_string(i),
                                 node->line_num,
                                 node->col_num,
                                 EXIT_TYPECHECK_FAILURE);
        }
    }

    for (auto &arg : node->args) {
        arg->accept(*this);
    }
}

void
TypecheckVisitor::visit_assign_node(AssignNode *node)
{
    // HACK: Avoid explicitly visiting the variable node, since the visitor does
    // not distinguish between visiting an expression on the left or right side
    // of an assignment.
    // TODO: Throw an AlbatrossError if the dynamic cast fails.
    auto lhs        = dynamic_cast<VarNode *>(node->lhs.get());
    lhs->value_type = lhs->var_info.value().var_type;

    // Typecheck lhs and rhs

    // Retrieve types
    Type type_rhs = typecheck_exp(node->rhs.get());
    Type type_lhs = lhs->value_type.value();

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
    auto var = dynamic_cast<VarNode *>(node->lhs.get());
    std::cout << "Variable written \"" << var->name << "\" type "
              << type_to_str(type_lhs) << "\n";
#endif
#endif
#endif
#endif

    if (type_lhs != type_rhs) {
        throw AlbatrossError("Mismatched types in assignment",
                             node->line_num,
                             node->col_num,
                             EXIT_TYPECHECK_FAILURE);
    }
}

void
TypecheckVisitor::visit_vardecl_node(VardeclNode *node)
{
    Type type_lhs = node->type;

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

    // Typecheck rhs
    Type type_rhs = typecheck_exp(node->rhs.get());

    if (type_lhs != type_rhs) {
        throw AlbatrossError("Mismatched types in variable declaration",
                             node->line_num,
                             node->col_num,
                             EXIT_TYPECHECK_FAILURE);
    }
    return;
}

void
TypecheckVisitor::visit_if_node(IfNode *node)
{
    // Typecheck cond
    Type cond_type = typecheck_exp(node->cond.get());

    if (cond_type != Type::Int) {
        throw AlbatrossError(
            "Condition expressions in if statements must be of type int, but got "
                + type_to_str(cond_type),
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    visit_stmts(node->then_stmts);
    visit_stmts(node->else_stmts);
}

void
TypecheckVisitor::visit_while_node(WhileNode *node)
{
    // Typecheck cond
    Type cond_type = typecheck_exp(node->cond.get());

    if (cond_type != Type::Int) {
        throw AlbatrossError(
            "Condition expressions in while statements must be of type int, but got "
                + type_to_str(cond_type),
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    visit_stmts(node->body_stmts);
    visit_stmts(node->otherwise_stmts);
}

void
TypecheckVisitor::visit_repeat_node(RepeatNode *node)
{
    // Typecheck cond
    Type cond_type = typecheck_exp(node->cond.get());

    if (cond_type != Type::Int) {
        throw AlbatrossError(
            "Condition expressions in repeat statements must be of type int, but got "
                + type_to_str(cond_type),
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    visit_stmts(node->body_stmts);
}

void
TypecheckVisitor::visit_call_stmt_node(CallStmtNode *node)
{
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
            "Incorrect number of arguments supplied for function " + node->name
                + ": expected " + std::to_string(n_params) + ", got "
                + std::to_string(n_args),
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    for (int i = 0; i < n_params; i++) {
        node->args[i]->accept(*this);
        Type arg_type   = typecheck_exp(node->args[i].get());
        Type param_type = info.params[i].type;

        if (arg_type != param_type) {
            throw AlbatrossError("Mismatched type in function " + node->name
                                     + " for param " + info.params[i].name
                                     + ", position " + std::to_string(i),
                                 node->line_num,
                                 node->col_num,
                                 EXIT_TYPECHECK_FAILURE);
        }
    }

    for (auto &arg : node->args) {
        arg->accept(*this);
    }
}

void
TypecheckVisitor::visit_ret_node(RetNode *node)
{
    // Assume ret exp has type void initially, but if a ret expression exists
    // then typecheck it and replace the type.
    Type ret_exp_type = Type::Void;

    if (node->ret_exp.has_value()) {
        ret_exp_type = typecheck_exp(node->ret_exp.value().get());
    }

    // If we are inside a function definition, the member variable fun_ret_type
    // will contain the declared return type of the function.
    if (fun_ret_type.has_value() && fun_ret_type.value() != ret_exp_type) {
        throw AlbatrossError(
            "Return statement does not return type specified in "
            "function declaration.",
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    // If the above check fails, we are in the global scope and can return only
    // integers.
    if (!fun_ret_type.has_value() && ret_exp_type != Type::Int) {
        throw AlbatrossError(
            "Return expression in global scope must be of type "
            "'int', but got '"
                + type_to_str(ret_exp_type) + "\'",
            node->line_num,
            node->col_num,
            EXIT_TYPECHECK_FAILURE);
    }

    // TODO: Can this take any other paths?
}

void
TypecheckVisitor::visit_fundec_node(FundecNode *node)
{
#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
#ifdef COMPILE_STAGE_TYPE_CHECKER
    std::cout << "Function declared \"" << node->name << "\" returns "
              << type_to_str(node->ret_type) << "\n";

    for (unsigned int i = 0; i < node->params.size(); i++) {
        auto &param = node->params[i];
        std::cout << "\tArgument \"" << param.name << "\" ";
        std::cout << "type " << type_to_str(param.type) << " ";
        std::cout << "position " << i << "\n";
    }
#endif
#endif
#endif
#endif
    fun_ret_type = node->ret_type;
    visit_stmts(node->body);
    fun_ret_type.reset();
}