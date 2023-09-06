#include "transform_ast.h"
#include "ast.h"
#include <vector>

// Try to fold an expression. Returns true if folding was performed, false if 
// not.
bool
fold_exp(std::unique_ptr<ExpNode> &exp)
{
        bool folded_something = false;

        switch (exp->kind) {
        case ExpNode::IntExp: break; // An int is left alone
        case ExpNode::StringExp: break; // Strings cannot be folded
        case ExpNode::VarExp: {
                // TODO: Check that the var is an integer constant. If it is,
                // switch this VarNode to an IntNode.
                break;
        }
        case ExpNode::BinopExp: {
                auto node = dynamic_cast<BinOpNode *>(exp.get());
                folded_something |= fold_exp(node->lhs);
                folded_something |= fold_exp(node->rhs);

                // After folding the children, it may be the case that both children
                // are now integer constants. In that case, apply the operation
                // contained in this BinOpNode and allocate a new IntNode with the
                // result.
                if (node->lhs->kind == ExpNode::IntExp
                    && node->rhs->kind == ExpNode::IntExp) {
                        int vlhs =
                                dynamic_cast<IntNode *>(node->lhs.get())->ival;
                        int vrhs =
                                dynamic_cast<IntNode *>(node->rhs.get())->ival;

                        auto res = new IntNode();
                        switch (node->op) {
                        case Operator::Or: res->ival = vlhs || vrhs; break;
                        case Operator::And: res->ival = vlhs && vrhs; break;
                        case Operator::Bor: res->ival = vlhs | vrhs; break;
                        case Operator::Xor: res->ival = vlhs ^ vrhs; break;
                        case Operator::Band: res->ival = vlhs & vrhs; break;
                        case Operator::Ne: res->ival = vlhs != vrhs; break;
                        case Operator::Eq: res->ival = vlhs == vrhs; break;
                        case Operator::Gt: res->ival = vlhs > vrhs; break;
                        case Operator::Ge: res->ival = vlhs >= vrhs; break;
                        case Operator::Lt: res->ival = vlhs < vrhs; break;
                        case Operator::Le: res->ival = vlhs <= vrhs; break;
                        case Operator::Plus: res->ival = vlhs + vrhs; break;
                        case Operator::Minus: res->ival = vlhs - vrhs; break;
                        case Operator::Times: res->ival = vlhs * vrhs; break;
                        case Operator::Div: res->ival = vlhs / vrhs; break;
                        case Operator::Rem: res->ival = vlhs % vrhs; break;
                        default: perror("Invalid operator"); exit(EXIT_FAILURE);
                        }

                        exp.reset(res);
                        folded_something = true;
                }
                break;
        }
        case ExpNode::UnopExp: {
                auto node = dynamic_cast<UnOpNode *>(exp.get());
                folded_something |= fold_exp(node->e);
                if (node->e->kind == ExpNode::IntExp) {
                        auto res = new IntNode();
                        int  v   = dynamic_cast<IntNode *>(node->e.get())->ival;
                        switch (node->op) {
                        case Operator::Not: res->ival = !v; break;
                        case Operator::Neg: res->ival = -v; break;
                        default: perror("Invalid operator"); exit(EXIT_FAILURE);
                        }

                        exp.reset(res);
                        folded_something = true;
                }
                break;
        }
        case ExpNode::CallExp: break;
        }

        return folded_something;
}

bool
fold_stmt(StmtNode *stmt)
{
        bool folded_something = false;
        switch (stmt->kind) {
        case StmtNode::VardeclStmt: {
                auto node = dynamic_cast<VardeclNode *>(stmt);
                folded_something |= fold_exp(node->rhs);
                break;
        }
        case StmtNode::AssignStmt: {
                auto node = dynamic_cast<AssignNode *>(stmt);
                folded_something |= fold_exp(node->rhs);
                break;
        }
        case StmtNode::IfStmt: {
                auto node = dynamic_cast<IfNode*>(stmt);
                folded_something |= fold_exp(node->cond);
                folded_something |= fold_stmts(node->then_stmts);
                folded_something |= fold_stmts(node->else_stmts);
                break;
        }
        case StmtNode::WhileStmt: {
                auto node = dynamic_cast<WhileNode*>(stmt);
                folded_something |= fold_exp(node->cond);
                folded_something |= fold_stmts(node->body_stmts);
                folded_something |= fold_stmts(node->otherwise_stmts);
                break;
        }
        case StmtNode::RepeatStmt: {
                auto node = dynamic_cast<RepeatNode*>(stmt);
                folded_something |= fold_exp(node->cond);
                folded_something |= fold_stmts(node->body_stmts); 
                break;
        }
        case StmtNode::CallStmt: {
                auto node = dynamic_cast<CallStmtNode*>(stmt);
                for (auto& arg : node->args) {
                        folded_something |= fold_exp(arg);
                }
                break;
        }
        case StmtNode::FundecStmt: {
                auto node = dynamic_cast<FundecNode*>(stmt);
                folded_something |= fold_stmts(node->body);
                break;
        }
        case StmtNode::RetStmt: {
                auto node = dynamic_cast<RetNode*>(stmt);
                if (node->ret_exp.has_value()) {
                        folded_something |= fold_exp(node->ret_exp.value());
                }
                break;
        }
        default: break;
        }

        return folded_something;
}

bool
fold_stmts(std::vector<std::unique_ptr<StmtNode>> &stmts)
{
        bool folded_something = false;
        for (auto &stmt : stmts) {
                folded_something |= fold_stmt(stmt.get());
        }
        return folded_something;
}