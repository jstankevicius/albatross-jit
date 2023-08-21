#pragma once

#include <deque>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "error.h"
#include "token.h"
#include "types.h"

typedef enum {
        Invalid,

        // Infix operators
        OpOr,
        OpAnd,
        OpBor,
        OpXor,
        OpBand,
        OpNe,
        OpEq,
        OpGt,
        OpGe,
        OpLt,
        OpLe,
        OpPlus,
        OpMinus,
        OpTimes,
        OpDiv,
        OpRem,

        // Prefix operators
        OpNot,
        OpNeg,

        // Postfix operators
        OpSub
} Operator;

static std::string
op_str(Operator op)
{
        switch (op) {
        case OpOr: return "||";
        case OpAnd: return "&&";
        case OpBor: return "|";
        case OpXor: return "^";
        case OpBand: return "&";
        case OpNe: return "<>";
        case OpEq: return "==";
        case OpGt: return ">";
        case OpGe: return ">=";
        case OpLt: return "<";
        case OpLe: return "<=";
        case OpPlus: return "+";
        case OpMinus: return "-";
        case OpTimes: return "*";
        case OpDiv: return "/";
        case OpRem: return "%";
        case OpNot: return "!";
        case OpNeg: return "-";
        case OpSub: return "[";
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

struct ExpNode {
        int line_num = -1;
        int col_num  = -1;

        enum ExpKind {
                IntExp,
                StringExp,
                VarExp,
                BinopExp,
                UnopExp,
                CallExp
        } kind;

        virtual std::string to_str() = 0;
        virtual ~ExpNode()
        {
        }
};

struct IntNode : ExpNode {
        int ival;

        IntNode()
        {
                kind = ExpKind::IntExp;
        }

        std::string to_str()
        {
                return "(" + std::to_string(ival) + ")";
        }
};

struct StrNode : ExpNode {
        std::string sval;

        StrNode()
        {
                kind = ExpKind::StringExp;
        }

        std::string to_str()
        {
                return "(\"" + sval + "\")";
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

        std::string to_str()
        {
                return "(" + op_str(op) + e->to_str() + ")";
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
                op = _op;
                lhs.swap(_lhs);
                rhs.swap(_rhs);
        }

        std::string to_str()
        {
                return "(" + lhs->to_str() + op_str(op) + rhs->to_str() + ")";
        }
};

struct VarNode : ExpNode {
        std::string            name;
        std::optional<VarInfo> info;

        VarNode()
        {
                kind = ExpKind::VarExp;
        }

        std::string to_str()
        {
                return "(" + name + ")";
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

        std::string to_str()
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
};

struct AssignNode : StmtNode {
        std::unique_ptr<ExpNode> lhs;
        std::unique_ptr<ExpNode> rhs;

        AssignNode()
        {
                kind = StmtKind::AssignStmt;
        }
};

struct VardeclNode : StmtNode {
        std::string              lhs;
        Type                     type;
        std::unique_ptr<ExpNode> rhs;

        VardeclNode()
        {
                kind = StmtKind::VardeclStmt;
        }
};

struct IfNode : StmtNode {
        std::unique_ptr<ExpNode>               cond;
        std::vector<std::unique_ptr<StmtNode>> then_stmts;
        std::vector<std::unique_ptr<StmtNode>> else_stmts;

        IfNode()
        {
                kind = StmtKind::IfStmt;
        }
};

struct WhileNode : StmtNode {
        std::unique_ptr<ExpNode>               cond;
        std::vector<std::unique_ptr<StmtNode>> body_stmts;
        std::vector<std::unique_ptr<StmtNode>> otherwise_stmts;

        WhileNode()
        {
                kind = StmtKind::WhileStmt;
        }
};

struct RepeatNode : StmtNode {
        std::unique_ptr<ExpNode>               cond;
        std::vector<std::unique_ptr<StmtNode>> body_stmts;

        RepeatNode()
        {
                kind = StmtKind::RepeatStmt;
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
};

struct FundecNode : StmtNode {
        std::string                            name;
        Type                                   ret_type;
        std::vector<ParamNode>                 params;
        std::vector<std::unique_ptr<StmtNode>> body;

        FundecNode()
        {
                kind = StmtKind::FundecStmt;
        }
};

struct RetNode : StmtNode {
        std::optional<std::unique_ptr<ExpNode>> ret_exp;

        RetNode()
        {
                kind = StmtKind::RetStmt;
        }
};