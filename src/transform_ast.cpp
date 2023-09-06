#include "transform_ast.h"
#include "ast.h"
#include <vector>

void
fold_exp(std::unique_ptr<ExpNode> &exp)
{
        switch (exp->kind) {
        case ExpNode::IntExp: return; // An int is left alone
        case ExpNode::StringExp: return; // Strings cannot be folded
        case ExpNode::VarExp: {
                // TODO: Check that the var is an integer constant. If it is,
                // switch this VarNode to an IntNode.
                return;
        }
        case ExpNode::BinopExp: {
                auto node = dynamic_cast<BinOpNode *>(exp.get());
                fold_exp(node->lhs);
                fold_exp(node->rhs);

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
                        case Operator::Or: {
                                res->ival = vlhs || vrhs;
                                break;
                        }
                        case Operator::And: {
                                res->ival = vlhs && vrhs;
                                break;
                        }
                        case Operator::Bor: {
                                res->ival = vlhs | vrhs;
                                break;
                        }
                        case Operator::Xor: {
                                res->ival = vlhs ^ vrhs;
                                break;
                        }
                        case Operator::Band: {
                                res->ival = vlhs & vrhs;
                                break;
                        }
                        case Operator::Ne: {
                                res->ival = vlhs != vrhs;
                                break;
                        }
                        case Operator::Eq: {
                                res->ival = vlhs == vrhs;
                                break;
                        }
                        case Operator::Gt: {
                                res->ival = vlhs > vrhs;
                                break;
                        }
                        case Operator::Ge: {
                                res->ival = vlhs >= vrhs;
                                break;
                        }
                        case Operator::Lt: {
                                res->ival = vlhs < vrhs;
                                break;
                        }
                        case Operator::Le: {
                                res->ival = vlhs <= vrhs;
                                break;
                        }
                        case Operator::Plus: {
                                res->ival = vlhs + vrhs;
                                break;
                        }
                        case Operator::Minus: {
                                res->ival = vlhs - vrhs;
                                break;
                        }
                        case Operator::Times: {
                                res->ival = vlhs * vrhs;
                                break;
                        }
                        case Operator::Div: {
                                res->ival = vlhs / vrhs;
                                break;
                        }
                        case Operator::Rem: {
                                res->ival = vlhs % vrhs;
                                break;
                        }
                        default: perror("Invalid operator"); exit(EXIT_FAILURE);
                        }

                        exp.reset(res);
                }
                return;
        }
        case ExpNode::UnopExp: {
                auto node = dynamic_cast<UnOpNode *>(exp.get());
                fold_exp(node->e);
                if (node->e->kind == ExpNode::IntExp) {
                        auto res = new IntNode();
                        int  v   = dynamic_cast<IntNode *>(node->e.get())->ival;
                        switch (node->op) {
                        case Operator::Not: {
                                res->ival = !v;
                                break;
                        }
                        case Operator::Neg: {
                                res->ival = -v;
                                break;
                        }
                        default: perror("Invalid operator"); exit(EXIT_FAILURE);
                        }

                        exp.reset(res);
                }
                return;
        }
        case ExpNode::CallExp: return;
        }
}

void fold_stmt(StmtNode* stmt) {
    switch (stmt->kind) {
        case StmtNode::VardeclStmt: {
            auto node = dynamic_cast<VardeclNode*>(stmt);
            fold_exp(node->rhs);
            std::cout << node->rhs->to_str() << "\n";
            break;
        }
        default: break;
    }
}

void fold_stmts(std::vector<std::unique_ptr<StmtNode>> &stmts) {
    for (auto& stmt : stmts) {
        fold_stmt(stmt.get());
    }
}