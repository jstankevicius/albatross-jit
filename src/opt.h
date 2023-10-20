#include "ast.h"

class OptimizerVisitor : public StmtVisitor, public ExpVisitor {
    private:
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

    public:
    void visit_stmts(std::list<std::unique_ptr<StmtNode>> &stmts) override;
    OptimizerVisitor();
};