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

  // TODO: This is probably not good, but for the sake of
  // error-handling there needs to be some way of determining the
  // stream the token originally came from. We obviously don't want
  // to make a copy of the stream for every token so we'll just take
  // a pointer to the stream instead. Probably want some other way
  // of getting the parent stream in the future. Thankfully fixing
  // this should only be a matter of fixing lexer.cpp, error.cpp,
  // and this file.
  std::string *stream;

  TokenType type;

  // TODO: more types eventually?
  std::string string_value;
};
