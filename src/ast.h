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

typedef struct ExpNode {
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

        typedef struct {
                Operator                 op;
                std::shared_ptr<ExpNode> e;
        } UnOps;

        typedef struct {
                Operator                 op;
                std::shared_ptr<ExpNode> lhs;
                std::shared_ptr<ExpNode> rhs;
        } BinOps;

        typedef struct {
                std::string                           name;
                std::vector<std::shared_ptr<ExpNode>> args;
                std::optional<FunInfo>                fun_info;
        } CallOps;

        typedef struct {
                std::string            name;
                std::optional<VarInfo> var_info;
        } VarOps;

        // TODO: Add IntrinsicOps
        std::variant<int, std::string, BinOps, UnOps, CallOps, VarOps> data;

        inline int int_ops()
        {
                return std::get<int>(data);
        }

        inline std::string &str_ops()
        {
                return std::get<std::string>(data);
        }

        inline BinOps &bin_ops()
        {
                return std::get<BinOps>(data);
        }

        inline UnOps &un_ops()
        {
                return std::get<UnOps>(data);
        }

        inline CallOps &call_ops()
        {
                return std::get<CallOps>(data);
        }

        inline VarOps &var_ops()
        {
                return std::get<VarOps>(data);
        }

} ExpNode;

typedef struct StmtNode {
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

        typedef struct {
                std::shared_ptr<ExpNode> lhs;
                std::shared_ptr<ExpNode> rhs;
        } AssignOps;

        typedef struct {
                std::string              lhs;
                Type                     type;
                std::shared_ptr<ExpNode> rhs;
        } VardeclOps;

        typedef struct {
                std::shared_ptr<ExpNode>               cond;
                std::vector<std::shared_ptr<StmtNode>> then_stmts;
                std::vector<std::shared_ptr<StmtNode>> else_stmts;
        } IfOps;

        typedef struct {
                std::shared_ptr<ExpNode>               cond;
                std::vector<std::shared_ptr<StmtNode>> body_stmts;
                std::vector<std::shared_ptr<StmtNode>> otherwise_stmts;
        } WhileOps;

        typedef struct {
                std::shared_ptr<ExpNode>               cond;
                std::vector<std::shared_ptr<StmtNode>> body_stmts;
        } RepeatOps;

        typedef struct {
                std::string                           name;
                std::vector<std::shared_ptr<ExpNode>> args;
                std::optional<FunInfo>                fun_info;
        } CallOps;

        typedef struct {
                std::string                            name;
                Type                                   ret_type;
                std::vector<ParamNode>                 params;
                std::vector<std::shared_ptr<StmtNode>> body;
        } FundecOps;

        typedef struct {
                std::shared_ptr<ExpNode> ret_exp;
        } RetOps;

        // TODO: Add IntrinsicStmt

        std::variant<VardeclOps,
                     AssignOps,
                     IfOps,
                     WhileOps,
                     RepeatOps,
                     CallOps,
                     FundecOps,
                     RetOps>
                data;

        inline AssignOps &assign_ops()
        {
                return std::get<AssignOps>(data);
        }
        inline VardeclOps &vardecl_ops()
        {
                return std::get<VardeclOps>(data);
        }
        inline IfOps &if_ops()
        {
                return std::get<IfOps>(data);
        }
        inline WhileOps &while_ops()
        {
                return std::get<WhileOps>(data);
        }
        inline RepeatOps &repeat_ops()
        {
                return std::get<RepeatOps>(data);
        }
        inline CallOps &call_ops()
        {
                return std::get<CallOps>(data);
        }
        inline FundecOps &fundec_ops()
        {
                return std::get<FundecOps>(data);
        }
        inline RetOps &ret_ops()
        {
                return std::get<RetOps>(data);
        }
} StmtNode;

std::shared_ptr<ExpNode>
new_int_exp_node(int ival);

std::shared_ptr<ExpNode>
new_unop_exp_node(Operator op, std::shared_ptr<ExpNode> e);

std::shared_ptr<ExpNode>
new_binop_exp_node(Operator                 op,
                   std::shared_ptr<ExpNode> lhs,
                   std::shared_ptr<ExpNode> rhs);

std::shared_ptr<ExpNode>
new_var_exp_node(std::string name);

std::shared_ptr<ExpNode>
new_str_exp_node(std::string str);

std::shared_ptr<ExpNode>
new_call_exp_node(std::string                            name,
                  std::vector<std::shared_ptr<ExpNode>> &args);

std::shared_ptr<StmtNode>
new_assign_stmt_node(std::shared_ptr<ExpNode> lhs,
                     std::shared_ptr<ExpNode> rhs);

std::shared_ptr<StmtNode>
new_if_stmt_node(std::shared_ptr<ExpNode>                cond,
                 std::vector<std::shared_ptr<StmtNode>> &then_stmts,
                 std::vector<std::shared_ptr<StmtNode>> &else_stmts);

std::shared_ptr<StmtNode>
new_while_stmt_node(std::shared_ptr<ExpNode>                cond,
                    std::vector<std::shared_ptr<StmtNode>> &body_stmts,
                    std::vector<std::shared_ptr<StmtNode>> &otherwise_stmts);

std::shared_ptr<StmtNode>
new_return_stmt_node(std::shared_ptr<ExpNode> ret_exp);

std::shared_ptr<StmtNode>
new_repeat_stmt_node(std::shared_ptr<ExpNode>                cond,
                     std::vector<std::shared_ptr<StmtNode>> &body_stmts);

std::shared_ptr<StmtNode>
new_fundec_stmt_node(std::string                            fun_name,
                     Type                                   ret_type,
                     std::vector<ParamNode>                &params,
                     std::vector<std::shared_ptr<StmtNode>> body);

std::shared_ptr<StmtNode>
new_vardecl_stmt_node(std::string              name,
                      Type                     type,
                      std::shared_ptr<ExpNode> rhs);

std::shared_ptr<StmtNode>
new_call_stmt_node(std::string                           name,
                   std::vector<std::shared_ptr<ExpNode>> args);