#include "parser.h"

#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

#include "ast.h"
#include "compiler_stages.h"
#include "error.h"
#include "lexer.h"
#include "token.h"

std::shared_ptr<Token>
expect_any_token(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto front = tokens.front();
        if (front->type == TokenType::Eof) {
                throw AlbatrossError("Unexpected EOF at end of file",
                                     front->line_num,
                                     front->col_num,
                                     EXIT_PARSER_FAILURE);
        }
        tokens.pop_front();
        return front;
}

// Expect the next token in the stream to have a particular type. If not, fail
// with an error on the token.
std::shared_ptr<Token>
expect_token_type(TokenType type, std::deque<std::shared_ptr<Token>> &tokens)
{
        auto front = tokens.front();

        if (front->type == TokenType::Eof) {
                throw AlbatrossError("Unexpected EOF at end of file",
                                     front->line_num,
                                     front->col_num,
                                     EXIT_PARSER_FAILURE);
        }

        if (front->type != type) {
                // TODO: better errors
                throw AlbatrossError("syntax error: unexpected token '"
                                             + front->string_value + "'",
                                     front->line_num,
                                     front->col_num,
                                     EXIT_PARSER_FAILURE);
        }

        tokens.pop_front();
        return front;
}

struct OpInfo {
        Operator op;

        int l_bp;
        int r_bp;

        enum { Prefix, Infix, Postfix, Invalid } kind;
};

// Return an operator's left and right binding power.
// The `minus_prefix_flag` parameter is not clean at all. This function should
// just take an operator and return its binding power. It shouldn't need to grab
// a token from a queue or do any checking of a flag to determine if a minus
// sign is actually a negative sign.
OpInfo
op_binding_power(TokenType op, bool minus_prefix_flag = false)
{
        // In decreasing order of precedence:
        switch (op) {
        case TokenType::Lbracket:
                return OpInfo{ Operator::OpSub, 200, -1, OpInfo::Postfix };
        case TokenType::OpNot:
                return OpInfo{ Operator::OpNot, -1, 190, OpInfo::Prefix };
        case TokenType::OpTimes:
                return OpInfo{ Operator::OpTimes, 175, 180, OpInfo::Infix };
        case TokenType::OpDiv:
                return OpInfo{ Operator::OpDiv, 175, 180, OpInfo::Infix };
        case TokenType::OpRem:
                return OpInfo{ Operator::OpRem, 175, 180, OpInfo::Infix };
        case TokenType::OpPlus:
                return OpInfo{ Operator::OpPlus, 165, 170, OpInfo::Infix };
        case TokenType::OpMinus:
                return minus_prefix_flag ? OpInfo{ Operator::OpMinus,
                                                   -1,
                                                   190,
                                                   OpInfo::Prefix } :
                                           OpInfo{ Operator::OpMinus,
                                                   165,
                                                   170,
                                                   OpInfo::Infix };
        case TokenType::OpLt:
                return OpInfo{ Operator::OpLt, 145, 150, OpInfo::Infix };
        case TokenType::OpLe:
                return OpInfo{ Operator::OpLe, 145, 150, OpInfo::Infix };
        case TokenType::OpGt:
                return OpInfo{ Operator::OpGt, 145, 150, OpInfo::Infix };
        case TokenType::OpGe:
                return OpInfo{ Operator::OpGe, 145, 150, OpInfo::Infix };
        case TokenType::OpEq:
                return OpInfo{ Operator::OpEq, 135, 140, OpInfo::Infix };
        case TokenType::OpNe:
                return OpInfo{ Operator::OpNe, 135, 140, OpInfo::Infix };
        case TokenType::OpBand:
                return OpInfo{ Operator::OpBand, 125, 130, OpInfo::Infix };
        case TokenType::OpXor:
                return OpInfo{ Operator::OpXor, 115, 120, OpInfo::Infix };
        case TokenType::OpBor:
                return OpInfo{ Operator::OpBor, 105, 110, OpInfo::Infix };
        case TokenType::OpAnd:
                return OpInfo{ Operator::OpAnd, 95, 100, OpInfo::Infix };
        case TokenType::OpOr:
                return OpInfo{ Operator::OpOr, 85, 90, OpInfo::Infix };
        default: return OpInfo{ Operator::Invalid, -1, -1, OpInfo::Invalid };
        }
}

std::unique_ptr<ExpNode>
parse_var_exp(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok       = expect_token_type(TokenType::Identifier, tokens);
        auto name      = tok->string_value;
        auto node      = std::make_unique<VarNode>();
        node->name     = name;
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

std::unique_ptr<ExpNode>
parse_str_exp(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok       = expect_token_type(TokenType::StrLiteral, tokens);
        auto str       = tok->string_value;
        auto node      = std::make_unique<StrNode>();
        node->sval     = str;
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

std::unique_ptr<ExpNode>
parse_int_exp(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok       = expect_token_type(TokenType::IntLiteral, tokens);
        int  val       = std::atoi(tok->string_value.c_str());
        auto node      = std::make_unique<IntNode>();
        node->ival     = val;
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

std::unique_ptr<ExpNode>
parse_call_exp(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok  = expect_token_type(TokenType::Identifier, tokens);
        auto node = std::make_unique<CallNode>();
        auto name = tok->string_value;

        expect_token_type(TokenType::Lparen, tokens);
        std::vector<std::unique_ptr<ExpNode>> args;
        if (tokens.front()->type != TokenType::Rparen) {
                while (1) {
                        auto arg = parse_exp(tokens);
                        node->args.push_back(std::move(arg));
                        if (tokens.front()->type == TokenType::Comma) {
                                expect_token_type(TokenType::Comma, tokens);
                                continue;
                        } else {
                                expect_token_type(TokenType::Rparen, tokens);
                                break;
                        }
                }
        } else {
                expect_token_type(TokenType::Rparen, tokens);
        }

        node->name     = name;
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

// Pratt's parse() function. Recursively builds an expression AST from a token
// stream.
std::unique_ptr<ExpNode>
exp_bp(std::deque<std::shared_ptr<Token>> &tokens, int min_bp)
{
        std::unique_ptr<ExpNode> lhs;

        auto front = tokens.front();

        switch (front->type) {
        case TokenType::IntLiteral: {
                lhs = parse_int_exp(tokens);
                break;
        };
        case TokenType::Identifier: {
                // Check if this is a function call or just an identifier:
                if (tokens.at(1)->type == TokenType::Lparen) {
                        lhs = parse_call_exp(tokens);
                } else {
                        lhs = parse_var_exp(tokens);
                }

                break;
        }

        case TokenType::StrLiteral: {
                lhs = parse_str_exp(tokens);
                break;
        }

        case TokenType::Lparen: {
                expect_token_type(TokenType::Lparen, tokens);
                lhs = exp_bp(tokens, 0);
                expect_token_type(TokenType::Rparen, tokens);
                break;
        }

        case TokenType::OpMinus:
        case TokenType::OpNot: {
                OpInfo info = op_binding_power(tokens.front()->type, true);
                assert(info.kind == OpInfo::Prefix);
                auto r_bp = info.r_bp;

                // Consume the operator; it is guaranteed to be either OpMinus or OpNot
                auto tok = expect_any_token(tokens);
                auto rhs = exp_bp(tokens, r_bp);

                lhs           = std::make_unique<UnOpNode>(info.op, rhs);
                lhs->line_num = tok->line_num;
                lhs->col_num  = tok->col_num;
                break;
        }

        default:
                throw AlbatrossError("Expected an expression",
                                     front->line_num,
                                     front->col_num,
                                     EXIT_PARSER_FAILURE);
        }

        while (1) {
                // Check for EOF:
                if (tokens.front()->type == TokenType::Eof) {
                        break;
                }

                OpInfo info = op_binding_power(tokens.front()->type);
                if (info.kind == OpInfo::Postfix) {
                        int l_bp = info.l_bp;
                        if (l_bp < min_bp) {
                                break;
                        }

                        // Consume op token
                        auto tok = expect_any_token(tokens);

                        // new_binop_exp_node(op, lhs, rhs);
                        // TODO: implement [ operator
                        lhs = std::make_unique<UnOpNode>(info.op, lhs);
                        lhs->line_num = tok->line_num;
                        lhs->col_num  = tok->col_num;
                        continue;
                }

                if (info.kind == OpInfo::Infix) {
                        int l_bp = info.l_bp;
                        int r_bp = info.r_bp;
                        if (l_bp < min_bp) {
                                break;
                        }

                        // Consume op token
                        auto tok = expect_any_token(tokens);

                        // Now parse rhs
                        auto rhs = exp_bp(tokens, r_bp);
                        // TODO: cons rhs and lhs together
                        lhs = std::make_unique<BinOpNode>(info.op, lhs, rhs);
                        lhs->line_num = tok->line_num;
                        lhs->col_num  = tok->col_num;
                        continue;
                }

                // Anything that isn't an infix or postfix token (like a paren or semicolon)
                // will fall through to this `break` and exit the loop.
                break;
        }

        return lhs;
}

// Parse an expression from the token stream.
std::unique_ptr<ExpNode>
parse_exp(std::deque<std::shared_ptr<Token>> &tokens)
{
        return exp_bp(tokens, 0);
}

std::unique_ptr<StmtNode>
parse_vardecl_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        expect_token_type(TokenType::KeywordVar, tokens);

        auto tok  = expect_token_type(TokenType::Identifier, tokens);
        auto name = tok->string_value;
        auto type = str_to_type(
                expect_token_type(TokenType::TypeName, tokens)->string_value);
        expect_token_type(TokenType::Assign, tokens);
        std::unique_ptr<ExpNode> rhs = parse_exp(tokens);
        expect_token_type(TokenType::Semicolon, tokens);

        auto node      = std::make_unique<VardeclNode>();
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        node->type     = type;
        node->lhs      = name;
        node->rhs      = std::move(rhs);

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << node->rhs->to_str() << "\n";
#endif
#endif
#endif
#endif

        return node;
}

std::unique_ptr<StmtNode>
parse_assign_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        std::unique_ptr<ExpNode> lhs = parse_exp(tokens);
        auto tok = expect_token_type(TokenType::Assign, tokens);
        std::unique_ptr<ExpNode> rhs = parse_exp(tokens);
        expect_token_type(TokenType::Semicolon, tokens);

        auto node      = std::make_unique<AssignNode>();
        node->lhs      = std::move(lhs);
        node->rhs      = std::move(rhs);
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << node->rhs->to_str() << "\n";
#endif
#endif
#endif
#endif

        return node;
}

std::unique_ptr<StmtNode>
parse_return_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok = expect_token_type(TokenType::KeywordReturn, tokens);
        // TODO: This is bad.
        std::unique_ptr<ExpNode> ret_exp(nullptr);

        if (tokens.front()->type != TokenType::Semicolon) {
                ret_exp = parse_exp(tokens);

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
                std::cout << ret_exp->to_str() << "\n";
#endif
#endif
#endif
#endif
        }

        expect_token_type(TokenType::Semicolon, tokens);

        auto node      = std::make_unique<RetNode>();
        node->ret_exp  = std::move(ret_exp);
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;

        return node;
}

std::unique_ptr<StmtNode>
parse_if_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok  = expect_token_type(TokenType::KeywordIf, tokens);
        auto node = std::make_unique<IfNode>();

        // Should parentheses be optional?
        node->cond = parse_exp(tokens);

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << node->cond->to_str() << "\n";
#endif
#endif
#endif
#endif

        expect_token_type(TokenType::Lcurl, tokens);
        while (tokens.front()->type != TokenType::Rcurl) {
                node->then_stmts.push_back(parse_stmt(tokens));
        }
        expect_token_type(TokenType::Rcurl, tokens);

        // TODO: Add an EOF token so we don't have to do this kind of dumb checking.
        if (!tokens.empty() && tokens.front()->type == TokenType::KeywordElse) {
                expect_token_type(TokenType::KeywordElse, tokens);
                expect_token_type(TokenType::Lcurl, tokens);

                while (tokens.front()->type != TokenType::Rcurl) {
                        node->else_stmts.push_back(parse_stmt(tokens));
                }

                expect_token_type(TokenType::Rcurl, tokens);
        }

        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;

        return node;
}

std::unique_ptr<StmtNode>
parse_while_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok   = expect_token_type(TokenType::KeywordWhile, tokens);
        auto node  = std::make_unique<WhileNode>();
        node->cond = parse_exp(tokens);
#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << node->cond->to_str() << "\n";
#endif
#endif
#endif
#endif

        expect_token_type(TokenType::Lcurl, tokens);
        while (tokens.front()->type != TokenType::Rcurl) {
                node->body_stmts.push_back(parse_stmt(tokens));
        }
        expect_token_type(TokenType::Rcurl, tokens);

        if (!tokens.empty()
            && tokens.front()->type == TokenType::KeywordOtherwise) {
                expect_token_type(TokenType::KeywordOtherwise, tokens);
                expect_token_type(TokenType::Lcurl, tokens);
                while (tokens.front()->type != TokenType::Rcurl) {
                        node->otherwise_stmts.push_back(parse_stmt(tokens));
                }
                expect_token_type(TokenType::Rcurl, tokens);
        }

        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;

        return node;
}

std::unique_ptr<StmtNode>
parse_repeat_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok   = expect_token_type(TokenType::KeywordRepeat, tokens);
        auto node  = std::make_unique<RepeatNode>();
        node->cond = parse_exp(tokens);

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << node->cond->to_str() << "\n";
#endif
#endif
#endif
#endif

        expect_token_type(TokenType::Lcurl, tokens);

        while (tokens.front()->type != TokenType::Rcurl) {
                node->body_stmts.push_back(parse_stmt(tokens));
        }

        expect_token_type(TokenType::Rcurl, tokens);
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

std::unique_ptr<StmtNode>
parse_fundecl_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto tok  = expect_token_type(TokenType::KeywordFun, tokens);
        auto node = std::make_unique<FundecNode>();
        auto fun_name =
                expect_token_type(TokenType::Identifier, tokens)->string_value;

        // TODO: We can (maybe) make type declarations optional for functions.
        // Instead, infer from the types of all return statements in the function.
        auto type = str_to_type(
                expect_token_type(TokenType::TypeName, tokens)->string_value);
        expect_token_type(TokenType::Lparen, tokens);

        // TODO: Should this be a pointer? Like ParamNode_p?
        std::vector<ParamNode> params;
        if (tokens.front()->type != TokenType::Rparen) {
                while (1) {
                        auto param_name =
                                expect_token_type(TokenType::Identifier, tokens)
                                        ->string_value;
                        auto param_type =
                                expect_token_type(TokenType::TypeName, tokens)
                                        ->string_value;

                        params.push_back(ParamNode{ param_name,
                                                    str_to_type(param_type) });

                        if (tokens.front()->type == TokenType::Comma) {
                                expect_token_type(TokenType::Comma, tokens);
                                continue;
                        } else {
                                expect_token_type(TokenType::Rparen, tokens);
                                break;
                        }
                }
        } else {
                expect_token_type(TokenType::Rparen, tokens);
        }
        expect_token_type(TokenType::Lcurl, tokens);
        while (tokens.front()->type != TokenType::Rcurl) {
                auto stmt = parse_stmt(tokens);
                node->body.push_back(std::move(stmt));
        }
        expect_token_type(TokenType::Rcurl, tokens);

        node->name     = fun_name;
        node->ret_type = type;
        node->params   = params;
        node->line_num = tok->line_num;
        node->col_num  = tok->col_num;
        return node;
}

std::unique_ptr<StmtNode>
parse_call_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        auto token    = expect_token_type(TokenType::Identifier, tokens);
        auto name     = token->string_value;
        auto line_num = token->line_num;
        auto col_num  = token->col_num;
        auto node     = std::make_unique<CallStmtNode>();

        expect_token_type(TokenType::Lparen, tokens);

        if (tokens.front()->type != TokenType::Rparen) {
                while (1) {
                        node->args.push_back(parse_exp(tokens));
                        if (tokens.front()->type == TokenType::Comma) {
                                expect_token_type(TokenType::Comma, tokens);
                                continue;
                        } else {
                                expect_token_type(TokenType::Rparen, tokens);
                                break;
                        }
                }

        } else {
                expect_token_type(TokenType::Rparen, tokens);
        }

        expect_token_type(TokenType::Semicolon, tokens);

        node->name     = name;
        node->line_num = line_num;
        node->col_num  = col_num;

#ifdef COMPILE_STAGE_LEXER
#ifdef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
        std::cout << name << "(";
        std::string arg_str = "";
        for (unsigned int i = 0; i < node->args.size(); i++) {
                arg_str += node->args[i]->to_str();
                if (i < node->args.size() - 1) {
                        arg_str += ",";
                }
        }
        std::cout << arg_str;

        // NOTE: This SHOULD print a newline, but due to a bug in the test-generating
        // script, call statements do not emit a newline character at the end. I am
        // keeping the output in its original form for "compatibility".
        // The correct line would be `std::cout << ")\n";`
        std::cout << ")";
#endif
#endif
#endif
#endif

        return node;
}

std::unique_ptr<StmtNode>
parse_stmt(std::deque<std::shared_ptr<Token>> &tokens)
{
        // Parse a top-level statement and return its AST.
        const auto front = tokens.front();

        switch (front->type) {
        case TokenType::Identifier: {
                if (tokens.at(1)->type != TokenType::Lparen) {
                        return parse_assign_stmt(tokens);
                } else {
                        return parse_call_stmt(tokens);
                }
        }
        case TokenType::KeywordVar: return parse_vardecl_stmt(tokens);
        case TokenType::KeywordReturn: return parse_return_stmt(tokens);
        case TokenType::KeywordIf: return parse_if_stmt(tokens);
        case TokenType::KeywordWhile: return parse_while_stmt(tokens);
        case TokenType::KeywordRepeat: return parse_repeat_stmt(tokens);
        case TokenType::KeywordFun: return parse_fundecl_stmt(tokens);
        default:
                throw AlbatrossError("expected a statement",
                                     front->line_num,
                                     front->col_num,
                                     EXIT_PARSER_FAILURE);
        }
}

std::vector<std::unique_ptr<StmtNode>>
parse_stmts(std::deque<std::shared_ptr<Token>> &tokens)
{
        std::vector<std::unique_ptr<StmtNode>> stmts;
        while (tokens.front()->type != TokenType::Eof) {
                stmts.push_back(parse_stmt(tokens));
        }
        return stmts;
}