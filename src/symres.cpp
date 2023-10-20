#include "symres.h"

#include "error.h"

#include <algorithm>
#include <iostream>

void
SymbolResolverVisitor::visit_int_node(IntNode *node){
    // Nothing to do.
};

void
SymbolResolverVisitor::visit_string_node(StrNode *node){
    // Nothing to do.
};

void
SymbolResolverVisitor::visit_var_node(VarNode *node)
{
    auto res = vars.find_symbol(node->name);

    // If res has no value, there was no corresponding vardecl that declared
    // this variable. Error out:
    if (!res.has_value()) {
        throw AlbatrossError("Could not find symbol " + node->name,
                             node->line_num,
                             node->col_num,
                             EXIT_SYMRES_FAILURE);
    }

    // Otherwise, update this var node with the type and index.
    node->var_info = res;
}

void
SymbolResolverVisitor::visit_binop_node(BinOpNode *node)
{
    node->lhs->accept(*this);
    node->rhs->accept(*this);
}

void
SymbolResolverVisitor::visit_unop_node(UnOpNode *node)
{
    node->e->accept(*this);
}

void
SymbolResolverVisitor::visit_call_node(CallNode *node)
{
    auto res = functions.find_symbol(node->name);

    if (!res.has_value()) {
        throw AlbatrossError("Undefined function " + node->name,
                             node->line_num,
                             node->col_num,
                             EXIT_SYMRES_FAILURE);
    }

    // If the function exists, resolve its args:
    for (auto &arg : node->args) {
        arg->accept(*this);
    }

    node->fun_info = res;
}

void
SymbolResolverVisitor::visit_assign_node(AssignNode *node)
{
    node->lhs->accept(*this);
    node->rhs->accept(*this);
}

void
SymbolResolverVisitor::visit_vardecl_node(VardeclNode *node)
{
    Type  type = node->type;
    auto &name = node->lhs;

    // Check that we are not redeclaring the variable.
    if (vars.cur_scope()->find_symbol(name)) {
        throw AlbatrossError("Redefinition of variable " + name,
                             node->line_num,
                             node->col_num,
                             EXIT_SYMRES_FAILURE);
    }

    node->rhs->accept(*this);

    // Construct a VarInfo struct for this variable.
    vars.add_symbol(name, VarInfo{ type, vars.sym_idx });
}

void
SymbolResolverVisitor::visit_if_node(IfNode *node)
{
    node->cond->accept(*this);
    vars.enter_scope();
    visit_stmts(node->then_stmts);
    vars.exit_scope();

    vars.enter_scope();
    visit_stmts(node->else_stmts);
    vars.exit_scope();
}

void
SymbolResolverVisitor::visit_while_node(WhileNode *node)
{
    node->cond->accept(*this);
    vars.enter_scope();
    visit_stmts(node->body_stmts);
    vars.exit_scope();

    vars.enter_scope();
    visit_stmts(node->otherwise_stmts);
    vars.exit_scope();
}

void
SymbolResolverVisitor::visit_repeat_node(RepeatNode *node)
{
    node->cond->accept(*this);
    vars.enter_scope();
    visit_stmts(node->body_stmts);
    vars.exit_scope();
}

void
SymbolResolverVisitor::visit_call_stmt_node(CallStmtNode *node)
{
    auto res = functions.find_symbol(node->name);

    if (!res.has_value()) {
        throw AlbatrossError("Undefined function " + node->name,
                             node->line_num,
                             node->col_num,
                             EXIT_SYMRES_FAILURE);
    }

    for (auto &arg : node->args) {
        arg->accept(*this);
    }
    node->fun_info = res;
}

void
SymbolResolverVisitor::visit_fundec_node(FundecNode *node)
{
    // Make sure we're not redeclaring a function.
    if (functions.cur_scope()->find_symbol(node->name).has_value()) {
        throw AlbatrossError("Redefinition of function " + node->name,
                             node->line_num,
                             node->col_num,
                             EXIT_SYMRES_FAILURE);
    }

    // TODO: Constructing FunInfo this way is bad. Make an actual constructor.
    functions.add_symbol(
        node->name, FunInfo{ node->ret_type, functions.sym_idx, node->params });
    vars.enter_scope();

    // Add parameters into the scope of the function body.
    for (auto &p : node->params) {
        vars.add_symbol(p.name, VarInfo{ p.type, vars.sym_idx });
    }

    visit_stmts(node->body);
    vars.exit_scope();
}

void
SymbolResolverVisitor::visit_ret_node(RetNode *node)
{
    if (node->ret_exp.has_value()) {
        node->ret_exp.value()->accept(*this);
    }
}

void
SymbolResolverVisitor::visit_stmts(std::list<std::unique_ptr<StmtNode>> &stmts)
{
    for (auto &stmt : stmts) {
        stmt->accept(*this);
    }
}