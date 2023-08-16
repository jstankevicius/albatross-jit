#include <memory>

#include "ast.h"
#include "token.h"

template <class T> inline auto assign_to(T &dst)
{
        return [&dst](auto const &src) { dst = src; };
}

template <typename Ops, typename... Vals>
ExpNode_p _new_exp_node(ExpNode::ExpKind kind, Vals... vals)
{
        ExpNode_p p   = std::make_shared<ExpNode>();
        ExpOps    ops = Ops{ vals... };
        p->kind       = kind;

        std::visit(assign_to(p->data), ops);
        return p;
}

ExpNode_p new_var_exp_node(std::string name)
{
        return _new_exp_node<ExpNode::VarOps, std::string,
                             std::optional<VarInfo>>(ExpNode::VarExp, name, {});
}

ExpNode_p new_str_exp_node(std::string str)
{
        return _new_exp_node<std::string, std::string>(ExpNode::StringExp, str);
}

ExpNode_p new_int_exp_node(int ival)
{
        return _new_exp_node<int>(ExpNode::IntExp, ival);
}

ExpNode_p new_binop_exp_node(Operator op, ExpNode_p lhs, ExpNode_p rhs)
{
        return _new_exp_node<ExpNode::BinOps, Operator, ExpNode_p, ExpNode_p>(
                ExpNode::BinopExp, op, lhs, rhs);
}

ExpNode_p new_unop_exp_node(Operator op, ExpNode_p e)
{
        return _new_exp_node<ExpNode::UnOps, Operator, ExpNode_p>(
                ExpNode::UnopExp, op, e);
}

ExpNode_p new_call_exp_node(std::string name, std::vector<ExpNode_p> &args)
{
        return _new_exp_node<ExpNode::CallOps, std::string,
                             std::vector<ExpNode_p> &, std::optional<FunInfo>>(
                ExpNode::CallExp, name, args, {});
}

StmtNode_p new_assign_stmt_node(ExpNode_p lhs, ExpNode_p rhs)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::AssignOps{ lhs, rhs };
        p->kind     = StmtNode::AssignStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

StmtNode_p new_vardecl_stmt_node(std::string name, Type type, ExpNode_p rhs)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::VardeclOps{ name, type, rhs };
        p->kind     = StmtNode::VardeclStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

StmtNode_p new_return_stmt_node(ExpNode_p ret_exp)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::RetOps{ ret_exp };
        p->kind     = StmtNode::RetStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

StmtNode_p new_if_stmt_node(ExpNode_p                cond,
                            std::vector<StmtNode_p> &then_stmts,
                            std::vector<StmtNode_p> &else_stmts)
{
        auto    p       = std::make_shared<StmtNode>();
        StmtOps if_stmt = StmtNode::IfOps{ cond, then_stmts, else_stmts };
        p->kind         = StmtNode::IfStmt;
        std::visit(assign_to(p->data), if_stmt);
        return p;
}

StmtNode_p new_while_stmt_node(ExpNode_p                cond,
                               std::vector<StmtNode_p> &body_stmts,
                               std::vector<StmtNode_p> &otherwise_stmts)
{
        auto    p = std::make_shared<StmtNode>();
        StmtOps while_stmt =
                StmtNode::WhileOps{ cond, body_stmts, otherwise_stmts };
        p->kind = StmtNode::WhileStmt;
        std::visit(assign_to(p->data), while_stmt);
        return p;
}

StmtNode_p new_repeat_stmt_node(ExpNode_p                cond,
                                std::vector<StmtNode_p> &body_stmts)
{
        auto    p           = std::make_shared<StmtNode>();
        StmtOps repeat_stmt = StmtNode::RepeatOps{ cond, body_stmts };
        p->kind             = StmtNode::RepeatStmt;
        std::visit(assign_to(p->data), repeat_stmt);
        return p;
}

StmtNode_p new_fundec_stmt_node(std::string             fun_name,
                                Type                    ret_type,
                                std::vector<TypeNode>  &params,
                                std::vector<StmtNode_p> body)
{
        auto    p = std::make_shared<StmtNode>();
        StmtOps repeat_stmt =
                StmtNode::FundecOps{ fun_name, ret_type, params, body };
        p->kind = StmtNode::FundecStmt;
        std::visit(assign_to(p->data), repeat_stmt);
        return p;
}

StmtNode_p new_call_stmt_node(std::string name, std::vector<ExpNode_p> args)
{
        auto    p         = std::make_shared<StmtNode>();
        StmtOps call_stmt = StmtNode::CallOps{ name, args, {} };
        p->kind           = StmtNode::CallStmt;
        std::visit(assign_to(p->data), call_stmt);
        return p;
}