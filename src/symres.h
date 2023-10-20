#pragma once

#include "ast.h"
#include "symtab.h"
#include "types.h"

class SymbolResolverVisitor : public StmtVisitor, public ExpVisitor {
private:
    SymbolTable<VarInfo> vars;
    SymbolTable<FunInfo> functions;

public:
    void visit_int_node(IntNode *node) override;
    void visit_string_node(StrNode *node) override;
    void visit_var_node(VarNode *node) override;
    void visit_binop_node(BinOpNode *node) override;
    void visit_unop_node(UnOpNode *node) override;
    void visit_call_node(CallNode *node) override;

    void visit_assign_node(AssignNode *node) override;
    void visit_vardecl_node(VardeclNode *node) override;
    void visit_if_node(IfNode *node) override;
    void visit_while_node(WhileNode *node) override;
    void visit_repeat_node(RepeatNode *node) override;
    void visit_call_stmt_node(CallStmtNode *node) override;
    void visit_fundec_node(FundecNode *node) override;
    void visit_ret_node(RetNode *node) override;

    void visit_stmts(std::list<std::unique_ptr<StmtNode>> &stmts) override;

    SymbolResolverVisitor()
    {
        vars.enter_scope();
        functions.enter_scope();
    }

    ~SymbolResolverVisitor()
    {
    }
};