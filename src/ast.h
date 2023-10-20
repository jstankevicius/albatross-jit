#pragma once

#include <deque>
#include <list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "error.h"
#include "token.h"
#include "types.h"

enum class Operator {
    Invalid,

    // Infix operators
    Or,
    And,
    Bor,
    Xor,
    Band,
    Ne,
    Eq,
    Gt,
    Ge,
    Lt,
    Le,
    Add,
    Sub,
    Mul,
    Div,
    Rem,

    // Prefix operators
    Not,
    Neg,
};

static std::string
op_str(Operator op)
{
    switch (op) {
    case Operator::Or: return "||";
    case Operator::And: return "&&";
    case Operator::Bor: return "|";
    case Operator::Xor: return "^";
    case Operator::Band: return "&";
    case Operator::Ne: return "<>";
    case Operator::Eq: return "==";
    case Operator::Gt: return ">";
    case Operator::Ge: return ">=";
    case Operator::Lt: return "<";
    case Operator::Le: return "<=";
    case Operator::Add: return "+";
    case Operator::Sub: return "-";
    case Operator::Mul: return "*";
    case Operator::Div: return "/";
    case Operator::Rem: return "%";
    case Operator::Not: return "!";
    case Operator::Neg: return "-";
    default: perror("Invalid operator"); exit(EXIT_FAILURE);
    }
}

typedef struct {
    std::string name;
    Type        type;
} ParamNode;

typedef struct {
    Type var_type;
    int  var_idx;
} VarInfo;

typedef struct {
    Type                   ret_type;
    int                    var_idx_db;
    std::vector<ParamNode> params;
} FunInfo;

struct ExpNode;
struct IntNode;
struct StrNode;
struct VarNode;
struct BinOpNode;
struct UnOpNode;
struct CallNode;

class ExpVisitor {
public:
    virtual void visit_int_node(IntNode *node)     = 0;
    virtual void visit_string_node(StrNode *node)  = 0;
    virtual void visit_var_node(VarNode *node)     = 0;
    virtual void visit_binop_node(BinOpNode *node) = 0;
    virtual void visit_unop_node(UnOpNode *node)   = 0;
    virtual void visit_call_node(CallNode *node)   = 0;
};

struct ExpNode {
    int line_num = -1;
    int col_num  = -1;

    unsigned int reg = -1;

    std::optional<Type> value_type;
    std::optional<int>  constprop_value;

    enum ExpKind { IntExp, StringExp, VarExp, BinopExp, UnopExp, CallExp } kind;

    virtual std::string to_str() = 0;
    virtual ~ExpNode()
    {
    }

    virtual void accept(ExpVisitor &visitor) = 0;
};

struct IntNode : ExpNode {
    int ival;

    IntNode()
    {
        kind = ExpKind::IntExp;
    }

    std::string to_str() override
    {
        return "(" + std::to_string(ival) + ")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_int_node(this);
    }
};

struct StrNode : ExpNode {
    std::string sval;

    StrNode()
    {
        kind = ExpKind::StringExp;
    }

    std::string to_str() override
    {
        return "(\"" + sval + "\")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_string_node(this);
    }
};

struct UnOpNode : ExpNode {
    Operator                 op;
    std::unique_ptr<ExpNode> e;

    UnOpNode()
    {
        kind = ExpKind::UnopExp;
    }

    UnOpNode(Operator _op, std::unique_ptr<ExpNode> &_e)
    {
        kind = ExpKind::UnopExp;
        op   = _op;
        e    = std::move(_e);
    }

    std::string to_str() override
    {
        return "(" + op_str(op) + e->to_str() + ")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_unop_node(this);
    }
};

struct BinOpNode : ExpNode {
    Operator                 op;
    std::unique_ptr<ExpNode> lhs;
    std::unique_ptr<ExpNode> rhs;

    BinOpNode()
    {
        kind = ExpKind::BinopExp;
    }

    BinOpNode(Operator                  _op,
              std::unique_ptr<ExpNode> &_lhs,
              std::unique_ptr<ExpNode> &_rhs)
    {
        kind = ExpKind::BinopExp;
        op   = _op;
        lhs.swap(_lhs);
        rhs.swap(_rhs);
    }

    std::string to_str() override
    {
        return "(" + lhs->to_str() + op_str(op) + rhs->to_str() + ")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_binop_node(this);
    }
};

struct VarNode : ExpNode {
    std::string            name;
    std::optional<VarInfo> var_info;

    VarNode()
    {
        kind = ExpKind::VarExp;
    }

    std::string to_str() override
    {
        return "(" + name + ")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_var_node(this);
    }
};

struct CallNode : ExpNode {
    std::string                           name;
    std::vector<std::unique_ptr<ExpNode>> args;
    std::optional<FunInfo>                fun_info;

    CallNode()
    {
        kind = ExpKind::CallExp;
    }

    std::string to_str() override
    {
        std::string arg_str = "";
        for (unsigned int i = 0; i < args.size(); i++) {
            arg_str += args[i]->to_str();
            if (i < args.size() - 1) {
                arg_str += ",";
            }
        }

        return name + "(" + arg_str + ")";
    }

    void accept(ExpVisitor &visitor) override
    {
        visitor.visit_call_node(this);
    }
};

struct StmtNode;
struct AssignNode;
struct VardeclNode;
struct IfNode;
struct WhileNode;
struct RepeatNode;
struct CallStmtNode;
struct FundecNode;
struct RetNode;

class StmtVisitor {
public:
    virtual void visit_assign_node(AssignNode *node)      = 0;
    virtual void visit_vardecl_node(VardeclNode *node)    = 0;
    virtual void visit_if_node(IfNode *node)              = 0;
    virtual void visit_while_node(WhileNode *node)        = 0;
    virtual void visit_repeat_node(RepeatNode *node)      = 0;
    virtual void visit_call_stmt_node(CallStmtNode *node) = 0;
    virtual void visit_fundec_node(FundecNode *node)      = 0;
    virtual void visit_ret_node(RetNode *node)            = 0;

    virtual void visit_stmts(std::list<std::unique_ptr<StmtNode>> &stmts) = 0;
};

struct StmtNode {
    int line_num = -1;
    int col_num  = -1;

    enum StmtKind {
        AssignStmt,
        VardeclStmt,
        IfStmt,
        WhileStmt,
        RepeatStmt,
        CallStmt,
        FundecStmt,
        RetStmt
    } kind;

    virtual ~StmtNode()                       = default;
    virtual void accept(StmtVisitor &visitor) = 0;
};

struct AssignNode : StmtNode {
    std::unique_ptr<ExpNode> lhs;
    std::unique_ptr<ExpNode> rhs;

    AssignNode()
    {
        kind = StmtKind::AssignStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_assign_node(this);
    }
};

struct VardeclNode : StmtNode {
    Type                     type;
    std::string              lhs;
    std::unique_ptr<ExpNode> rhs;

    VardeclNode()
    {
        kind = StmtKind::VardeclStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_vardecl_node(this);
    }
};

struct IfNode : StmtNode {
    std::unique_ptr<ExpNode>             cond;
    std::list<std::unique_ptr<StmtNode>> then_stmts;
    std::list<std::unique_ptr<StmtNode>> else_stmts;

    IfNode()
    {
        kind = StmtKind::IfStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_if_node(this);
    }
};

struct WhileNode : StmtNode {
    std::unique_ptr<ExpNode>             cond;
    std::list<std::unique_ptr<StmtNode>> body_stmts;
    std::list<std::unique_ptr<StmtNode>> otherwise_stmts;

    WhileNode()
    {
        kind = StmtKind::WhileStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_while_node(this);
    }
};

struct RepeatNode : StmtNode {
    std::unique_ptr<ExpNode>             cond;
    std::list<std::unique_ptr<StmtNode>> body_stmts;

    RepeatNode()
    {
        kind = StmtKind::RepeatStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_repeat_node(this);
    }
};

struct CallStmtNode : StmtNode {
    std::string                           name;
    std::vector<std::unique_ptr<ExpNode>> args;
    std::optional<FunInfo>                fun_info;

    CallStmtNode()
    {
        kind = StmtKind::CallStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_call_stmt_node(this);
    }
};

struct FundecNode : StmtNode {
    Type                                 ret_type;
    std::string                          name;
    std::vector<ParamNode>               params;
    std::list<std::unique_ptr<StmtNode>> body;

    FundecNode()
    {
        kind = StmtKind::FundecStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_fundec_node(this);
    }
};

struct RetNode : StmtNode {
    std::optional<std::unique_ptr<ExpNode>> ret_exp = std::nullopt;

    RetNode()
    {
        kind = StmtKind::RetStmt;
    }

    void accept(StmtVisitor &visitor) override
    {
        visitor.visit_ret_node(this);
    }
};
