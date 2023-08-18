#include <memory>

#include "ast.h"
#include "token.h"

using StmtOps = std::variant<StmtNode::VardeclOps,
                             StmtNode::AssignOps,
                             StmtNode::IfOps,
                             StmtNode::WhileOps,
                             StmtNode::RepeatOps,
                             StmtNode::CallOps,
                             StmtNode::FundecOps,
                             StmtNode::RetOps>;

using ExpOps = std::variant<int,
                            std::string,
                            ExpNode::BinOps,
                            ExpNode::UnOps,
                            ExpNode::CallOps,
                            ExpNode::VarOps>;

template <class T>
inline auto
assign_to(T &dst)
{
        return [&dst](auto const &src) { dst = src; };
}

template <typename StructT, typename... Vals>
std::shared_ptr<ExpNode>
_new_exp_node(ExpNode::ExpKind kind, Vals... vals)
{
        std::shared_ptr<ExpNode> p   = std::make_shared<ExpNode>();
        ExpOps                   ops = StructT{ vals... };
        p->kind                      = kind;

        std::visit(assign_to(p->data), ops);
        return p;
}

template <typename StructT, typename... Vals>
std::shared_ptr<StmtNode>
_new_stmt_node(StmtNode::StmtKind kind, Vals... vals)
{
        std::shared_ptr<StmtNode> p   = std::make_shared<StmtNode>();
        StmtOps                   ops = StructT{ vals... };
        p->kind                       = kind;

        std::visit(assign_to(p->data), ops);
        return p;
}

std::shared_ptr<ExpNode>
new_var_exp_node(std::string name)
{
        return _new_exp_node<ExpNode::VarOps,
                             std::string,
                             std::optional<VarInfo>>(ExpNode::VarExp, name, {});
}

std::shared_ptr<ExpNode>
new_str_exp_node(std::string str)
{
        return _new_exp_node<std::string, std::string>(ExpNode::StringExp, str);
}

std::shared_ptr<ExpNode>
new_int_exp_node(int ival)
{
        return _new_exp_node<int>(ExpNode::IntExp, ival);
}

std::shared_ptr<ExpNode>
new_binop_exp_node(Operator                 op,
                   std::shared_ptr<ExpNode> lhs,
                   std::shared_ptr<ExpNode> rhs)
{
        return _new_exp_node<ExpNode::BinOps,
                             Operator,
                             std::shared_ptr<ExpNode>,
                             std::shared_ptr<ExpNode>>(
                ExpNode::BinopExp, op, lhs, rhs);
}

std::shared_ptr<ExpNode>
new_unop_exp_node(Operator op, std::shared_ptr<ExpNode> e)
{
        return _new_exp_node<ExpNode::UnOps, Operator, std::shared_ptr<ExpNode>>(
                ExpNode::UnopExp, op, e);
}

std::shared_ptr<ExpNode>
new_call_exp_node(std::string name, std::vector<std::shared_ptr<ExpNode>> &args)
{
        return _new_exp_node<ExpNode::CallOps,
                             std::string,
                             std::vector<std::shared_ptr<ExpNode>> &,
                             std::optional<FunInfo>>(
                ExpNode::CallExp, name, args, {});
}

std::shared_ptr<StmtNode>
new_assign_stmt_node(std::shared_ptr<ExpNode> lhs, std::shared_ptr<ExpNode> rhs)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::AssignOps{ lhs, rhs };
        p->kind     = StmtNode::AssignStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

std::shared_ptr<StmtNode>
new_vardecl_stmt_node(std::string name, Type type, std::shared_ptr<ExpNode> rhs)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::VardeclOps{ name, type, rhs };
        p->kind     = StmtNode::VardeclStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

std::shared_ptr<StmtNode>
new_return_stmt_node(std::shared_ptr<ExpNode> ret_exp)
{
        auto    p   = std::make_shared<StmtNode>();
        StmtOps ret = StmtNode::RetOps{ ret_exp };
        p->kind     = StmtNode::RetStmt;
        std::visit(assign_to(p->data), ret);
        return p;
}

std::shared_ptr<StmtNode>
new_if_stmt_node(std::shared_ptr<ExpNode>                cond,
                 std::vector<std::shared_ptr<StmtNode>> &then_stmts,
                 std::vector<std::shared_ptr<StmtNode>> &else_stmts)
{
        auto    p       = std::make_shared<StmtNode>();
        StmtOps if_stmt = StmtNode::IfOps{ cond, then_stmts, else_stmts };
        p->kind         = StmtNode::IfStmt;
        std::visit(assign_to(p->data), if_stmt);
        return p;
}

std::shared_ptr<StmtNode>
new_while_stmt_node(std::shared_ptr<ExpNode>                cond,
                    std::vector<std::shared_ptr<StmtNode>> &body_stmts,
                    std::vector<std::shared_ptr<StmtNode>> &otherwise_stmts)
{
        auto    p = std::make_shared<StmtNode>();
        StmtOps while_stmt =
                StmtNode::WhileOps{ cond, body_stmts, otherwise_stmts };
        p->kind = StmtNode::WhileStmt;
        std::visit(assign_to(p->data), while_stmt);
        return p;
}

std::shared_ptr<StmtNode>
new_repeat_stmt_node(std::shared_ptr<ExpNode>                cond,
                     std::vector<std::shared_ptr<StmtNode>> &body_stmts)
{
        auto    p           = std::make_shared<StmtNode>();
        StmtOps repeat_stmt = StmtNode::RepeatOps{ cond, body_stmts };
        p->kind             = StmtNode::RepeatStmt;
        std::visit(assign_to(p->data), repeat_stmt);
        return p;
}

std::shared_ptr<StmtNode>
new_fundec_stmt_node(std::string                            fun_name,
                     Type                                   ret_type,
                     std::vector<ParamNode>                &params,
                     std::vector<std::shared_ptr<StmtNode>> body)
{
        auto    p = std::make_shared<StmtNode>();
        StmtOps repeat_stmt =
                StmtNode::FundecOps{ fun_name, ret_type, params, body };
        p->kind = StmtNode::FundecStmt;
        std::visit(assign_to(p->data), repeat_stmt);
        return p;
}

std::shared_ptr<StmtNode>
new_call_stmt_node(std::string name, std::vector<std::shared_ptr<ExpNode>> args)
{
        auto    p         = std::make_shared<StmtNode>();
        StmtOps call_stmt = StmtNode::CallOps{ name, args, {} };
        p->kind           = StmtNode::CallStmt;
        std::visit(assign_to(p->data), call_stmt);
        return p;
}