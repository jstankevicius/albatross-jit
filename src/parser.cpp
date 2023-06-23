#include "parser.h"

#include <deque>
#include <iostream>
#include <memory>
#include <string>

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "token.h"

// Expect the next token in the stream to have a particular string as its
// contents. If not, fail with an error on the token.
void expect_token_string(std::string str,
                         std::deque<std::shared_ptr<Token>> &tokens)
{

    if (tokens.size() == 0)
    {
        printf("Unexpected EOF at end of file\n");
        exit(-1);
    }
    
    auto& token = tokens.front();
    if (token->string_value != str)
    {
        err_token(token,
                  "syntax error: expected '"
                  + str
                  + "', but got '"
                  + token->string_value
                  + "' ");
    }

    tokens.pop_front();
}

// Expect the next token in the stream to have a particular type. If not, fail
// with an error on the token.
void expect_token_type(TokenType type,
                       std::deque<std::shared_ptr<Token>> &tokens)
{
    if (tokens.size() == 0)
    {
        printf("Unexpected EOF at end of file\n");
        exit(-1);
    }

    switch (type) {
        default:
        break;
    }

    auto token = tokens.front();
    
    tokens.pop_front();
}

std::unique_ptr<StmtNode> parse_var(std::deque<std::shared_ptr<Token>>& tokens) {
    expect_token_type(TokenType::KeywordVar, tokens);
    expect_token_type(TokenType::Identifier, tokens);
    expect_token_type(TokenType::Assign, tokens);
}

std::unique_ptr<StmtNode> parse_stmt_or_var(std::deque<std::shared_ptr<Token>>& tokens) {
    // Parse a top-level statement and return its AST.
    
    while (!tokens.empty()) {
        auto token = tokens.front();
        switch (token->type) {
            case TokenType::KeywordVar: {
                break;
            }
            case TokenType::KeywordFun: {
                break;
            }
            default:
                err_token(token, "expected a variable declaration or a statement");
                break;
        }
        tokens.pop_front();
    }
}