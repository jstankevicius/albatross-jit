#pragma once

#include <memory>
#include <string>

enum class TokenType : unsigned char {
        Eof,

        Identifier,
        TypeName,

        KeywordIf,
        KeywordElse,
        KeywordWhile,
        KeywordReturn,
        KeywordOtherwise,
        KeywordRepeat,
        KeywordFun,
        KeywordVar,

        Semicolon,
        Comma,
        Lparen,
        Rparen,
        Lcurl,
        Rcurl,
        Lbracket,
        Rbracket,

        Assign,

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
        OpNot,

        // Literals
        IntLiteral,
        FloatLiteral,
        StrLiteral,
        BoolLiteral,
};

struct Token {
        int line_num, col_num;

        TokenType type;

        // TODO: more types eventually?
        std::string string_value;
};
