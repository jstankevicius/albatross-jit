#include "symres.h"
#include "ast.h"
#include "error.h"
#include <iostream>

void
resolve_exp(ExpNode              *exp,
            SymbolTable<VarInfo> &vars,
            SymbolTable<FunInfo> &functions)
{
    switch (exp->kind) {
    case ExpNode::IntExp: return;
    case ExpNode::StringExp: return;
    case ExpNode::VarExp: {
        auto node = dynamic_cast<VarNode *>(exp);
        auto res  = vars.find_symbol(node->name);

        // If res has no value, there was no corresponding vardecl that declared
        // this variable. Error out:
        if (!res.has_value()) {
            throw AlbatrossError("Could not find symbol " + node->name,
                                 exp->line_num,
                                 exp->col_num,
                                 EXIT_SYMRES_FAILURE);
        }

        // Otherwise, update this var node with the type and index.
        node->var_info = res;
        return;
    }
    case ExpNode::BinopExp: {
        auto node = dynamic_cast<BinOpNode *>(exp);
        resolve_exp(node->lhs.get(), vars, functions);
        resolve_exp(node->rhs.get(), vars, functions);
        return;
    }
    case ExpNode::UnopExp: {
        auto node = dynamic_cast<UnOpNode *>(exp);
        resolve_exp(node->e.get(), vars, functions);
        return;
    }
    case ExpNode::CallExp: {
        auto node = dynamic_cast<CallNode *>(exp);
        auto res  = functions.find_symbol(node->name);

        if (!res.has_value()) {
            throw AlbatrossError("Undefined function " + node->name,
                                 exp->line_num,
                                 exp->col_num,
                                 EXIT_SYMRES_FAILURE);
        }

        // If the function exists, resolve its args:
        for (auto &arg : node->args) {
            resolve_exp(arg.get(), vars, functions);
        }
        node->fun_info = res;

        return;
    }
    }
}

void
resolve_stmt(StmtNode             *stmt,
             SymbolTable<VarInfo> &vars,
             SymbolTable<FunInfo> &functions)
{
    switch (stmt->kind) {
    case StmtNode::VardeclStmt: {
        auto node = dynamic_cast<VardeclNode *>(stmt);

        Type  type = node->type;
        auto &name = node->lhs;

        // Check that we are not redeclaring the variable.
        if (vars.cur_scope()->find_symbol(name)) {
            throw AlbatrossError("Redefinition of variable " + name,
                                 stmt->line_num,
                                 stmt->col_num,
                                 EXIT_SYMRES_FAILURE);
        }

        resolve_exp(node->rhs.get(), vars, functions);

        // Construct a VarInfo struct for this variable.
        vars.add_symbol(name, VarInfo{ type, vars.sym_idx });
        return;
    }

    case StmtNode::AssignStmt: {
        auto node = dynamic_cast<AssignNode *>(stmt);
        resolve_exp(node->lhs.get(), vars, functions);
        resolve_exp(node->rhs.get(), vars, functions);
        return;
    }

    case StmtNode::IfStmt: {
        auto node = dynamic_cast<IfNode *>(stmt);

        resolve_exp(node->cond.get(), vars, functions);
        vars.enter_scope();
        resolve_stmts(node->then_stmts, vars, functions);
        vars.exit_scope();
        return;
    }

    case StmtNode::WhileStmt: {
        auto node = dynamic_cast<WhileNode *>(stmt);

        resolve_exp(node->cond.get(), vars, functions);
        vars.enter_scope();
        resolve_stmts(node->body_stmts, vars, functions);
        vars.exit_scope();

        vars.enter_scope();
        resolve_stmts(node->otherwise_stmts, vars, functions);
        vars.exit_scope();
        return;
    }

    case StmtNode::RepeatStmt: {
        auto node = dynamic_cast<RepeatNode *>(stmt);
        resolve_exp(node->cond.get(), vars, functions);

        vars.enter_scope();
        resolve_stmts(node->body_stmts, vars, functions);
        vars.exit_scope();
        return;
    }

    case StmtNode::CallStmt: {
        auto node = dynamic_cast<CallStmtNode *>(stmt);
        auto res  = functions.find_symbol(node->name);

        if (!res.has_value()) {
            throw AlbatrossError("Undefined function " + node->name,
                                 stmt->line_num,
                                 stmt->col_num,
                                 EXIT_SYMRES_FAILURE);
        }

        for (auto &arg : node->args) {
            resolve_exp(arg.get(), vars, functions);
        }
        node->fun_info = res;
        return;
    }

    case StmtNode::FundecStmt: {
        auto node = dynamic_cast<FundecNode *>(stmt);

        // Make sure we're not redeclaring a function.
        if (functions.cur_scope()->find_symbol(node->name).has_value()) {
            throw AlbatrossError("Redefinition of function " + node->name,
                                 stmt->line_num,
                                 stmt->col_num,
                                 EXIT_SYMRES_FAILURE);
        }

        // TODO: Constructing FunInfo this way is bad. Make an actual constructor.
        functions.add_symbol(
            node->name,
            FunInfo{ node->ret_type, functions.sym_idx, node->params });
        vars.enter_scope();

        // Add parameters into the scope of the function body.
        for (auto &p : node->params) {
            vars.add_symbol(p.name, VarInfo{ p.type, vars.sym_idx });
        }

        resolve_stmts(node->body, vars, functions);
        vars.exit_scope();

        return;
    }

    case StmtNode::RetStmt: {
        auto node = dynamic_cast<RetNode *>(stmt);
        if (node->ret_exp.has_value()) {
            resolve_exp(node->ret_exp.value().get(), vars, functions);
        }
        return;
    }
    }
}

void
resolve_stmts(std::list<std::unique_ptr<StmtNode>> &stmts,
              SymbolTable<VarInfo>                 &vars,
              SymbolTable<FunInfo>                 &functions)
{
    for (auto &stmt : stmts) {
        resolve_stmt(stmt.get(), vars, functions);
    }
}