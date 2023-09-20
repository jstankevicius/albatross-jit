#pragma once

#include "ast.h"
#include "symtab.h"
#include "types.h"


void
resolve_stmts(std::list<std::unique_ptr<StmtNode>> &stmts,
              SymbolTable<VarInfo>                 &vars,
              SymbolTable<FunInfo>                 &functions);

class SymbolResolverStmtVisitor;
class SymbolResolverExpVisitor : public ExpVisitor {
    SymbolResolverStmtVisitor &parent;
public:
    void visit_int_node(IntNode *node) override;
    void visit_string_node(StrNode *node) override;
    void visit_var_node(VarNode *node) override;
    void visit_binop_node(BinOpNode *node) override;
    void visit_unop_node(UnOpNode *node) override;
    void visit_call_node(CallNode *node) override;


    SymbolResolverExpVisitor(SymbolResolverStmtVisitor &parent_visitor)
        : parent(parent_visitor)
    {
    }

    ~SymbolResolverExpVisitor()
    {
    }
};

class SymbolResolverStmtVisitor : public StmtVisitor {
private:
    SymbolTable<VarInfo> vars;
    SymbolTable<FunInfo> functions;

    // Owning pointer to child expression visitor
    friend class SymbolResolverExpVisitor;
    std::unique_ptr<SymbolResolverExpVisitor> exp_visitor;
public:
    void visit_assign_node(AssignNode *node) override;
    void visit_vardecl_node(VardeclNode *node) override;
    void visit_if_node(IfNode *node) override;
    void visit_while_node(WhileNode *node) override;
    void visit_repeat_node(RepeatNode *node) override;
    void visit_call_stmt_node(CallStmtNode *node) override;
    void visit_fundec_node(FundecNode *node) override;
    void visit_ret_node(RetNode *node) override;

    SymbolResolverStmtVisitor()
    {
        exp_visitor = std::make_unique<SymbolResolverExpVisitor>(*this);
    }

    ~SymbolResolverStmtVisitor()
    {
    }
};