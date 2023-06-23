#pragma once

#include <memory>

typedef enum {
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
} BinOp;


typedef enum {
    OpNot,
    OpNeg
} UnOp;


typedef enum {
    IntType,
    StringType,
    VoidType
} Type;


typedef struct ExpNode {
    enum { IntExp, StringExp, BinopExp, UnopExp, CallExp, IntrinsicExp, VarExp } kind;
    union {
        int ival;
        std::string sval;

        struct { 
            BinOp op; 
            std::unique_ptr<struct ExpNode> e1; 
            std::unique_ptr<struct ExpNode> e2; 
        } bin_ops;

        struct { 
            UnOp op; 
            std::unique_ptr<struct ExpNode> e;
        } un_ops;

        struct {
            std::string name;
            std::vector<ExpNode> args;
        } call_ops;

        // TODO: Add intrinsic_ops

        struct { std::string name; } var_ops;

    } data;
} ExpNode;

typedef struct Param {
    Type type;
    std::string name;
} Param;

typedef struct FundecNode {
    std::string name;
    Type return_type;
    std::vector<ExpNode> args;
    // TODO: locs
    // TODO: stmts
} FundecNode;

typedef struct StmtNode {
    enum { AssignStmt, IfStmt, WhileStmt, RepeatStmt, RetStmt, CallStmt /* IntrinsicStmt */ } kind;
    union {
        struct {
            std::string lhs;
            ExpNode rhs;
        } assign_ops;

        struct {
            ExpNode cond;
            std::vector<StmtNode> then_stmts;
            std::vector<StmtNode> else_stmts;
        } if_ops;
    
        struct {
            ExpNode cond;
            std::vector<StmtNode> body_stmts;
            std::vector<StmtNode> otherwise_stmts;
        } while_ops;

        struct {
            ExpNode cond;
            std::vector<StmtNode> body_stmts;
        } repeat_ops;

        struct {
            std::string name;
            std::vector<ExpNode> args;
        } call_ops;

        struct {
            ExpNode ret_exp;
        } ret_ops;
        
        // TODO: Add intrinsics
    } data;
} StmtNode;