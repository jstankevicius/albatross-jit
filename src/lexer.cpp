// The Boba lexer. The lexer's only job is to take a "stream" (a fancy
// term for a string) and turn it into a list of tokens that is then
// passed back to the parser.

#include "lexer.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "error.h"

inline bool is_alpha(char c) {
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

inline bool is_numeric(char c) { return '0' <= c && c <= '9'; }

inline bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_alphanumeric(char c) { return is_numeric(c) || is_alpha(c); }

inline bool is_punctuation(char c) {
  return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
         c == ';' || c == ',';
}

// Returns whether the lexer has processed the entire stream. This happens when
// idx is pushed beyond the length of the actual stream.
inline bool ProgramText::done() { return idx >= stream.length(); }

// If the stream is not done, returns the character at stream_idx. Otherwise,
// returns -1 (an invalid character).
char ProgramText::cur_char() {
  if (!done()) {
    return stream[idx];
  }

  return -1;
}

// Peeks at the character directly after current character of the ProgramText.
// If reading this character would cause an out-of-bounds violation (i.e. there
// are no more characters left), returns -1.
char ProgramText::peek() {
  if (idx + 1 < stream.length()) {
    return stream[idx + 1];
  }

  return -1;
}

// If the lexer has not processed the entire stream yet, advances the
// ProgramText's idx to the next character, incrementing the current column
// number. If idx ends on a newline sequence (i.e. the character \n or the
// sequence \r\n), advance_char() skips over the newline and adjusts the line
// and column numbers accordingly.
void ProgramText::advance_char() {
  char cur = cur_char();

  if (!done()) {
    idx++;
    col_num++;

    if (cur == '\n' || (cur == '\r' && peek() == '\n')) {
      // If we went over to the next line, reset col_num to 1
      // and increment line_num.
      col_num = 1;
      line_num++;
    }
  }
}

// Skips over whitespace characters until a non-whitespace character is
// encountered.
void ProgramText::skip_whitespace() {
  while (is_whitespace(cur_char())) {
    advance_char();
  }
}

// Returns a token that is not a literal. These can be tokens like "+", ">=",
// "variable-name", etc. The lexer does not validate the names of the tokens, as
// that is done later.
std::shared_ptr<Token> get_symbol(ProgramText &t) {
  static std::unordered_map<std::string, TokenType> keyword_map;
  if (keyword_map.empty()) {
    keyword_map["if"] = TokenType::KeywordIf;
    keyword_map["else"] = TokenType::KeywordElse;
    keyword_map["while"] = TokenType::KeywordWhile;
    keyword_map["return"] = TokenType::KeywordReturn;
    keyword_map["otherwise"] = TokenType::KeywordOtherwise;
    keyword_map["repeat"] = TokenType::KeywordRepeat;
    keyword_map["fun"] = TokenType::KeywordFun;
  }

  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;

  std::string str;

  // Don't need to check for out of bounds since cur_char just
  // returns -1 once we've reached the end of the stream.
  // TODO: This doesn't support underscores
  while (!is_whitespace(t.cur_char()) && !t.done() &&
         is_alphanumeric(t.cur_char())) {
    str += t.cur_char();
    t.advance_char();
  }

  token->string_value = str;

  // TODO: formatting?
  token->type = keyword_map.find(str) != keyword_map.end()
                    ? keyword_map[str]
                    : TokenType::Identifier;
  return token;
}

// Returns a token for a numeric literal (like 123, 3.14, or their negative
// counterparts).
std::shared_ptr<Token> get_numeric_literal(ProgramText &t) {
  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;
  std::string num_literal;
  bool is_float_literal = false;

  while (is_numeric(t.cur_char())) {
    num_literal += t.cur_char();
    t.advance_char();
  }

  // Next character could potentially be a '.', which would make this a float
  // literal.
  if (t.cur_char() == '.' && is_numeric(t.peek())) {
    is_float_literal = true;
    num_literal += t.cur_char();
    t.advance_char();
  }

  else if (t.cur_char() == '.' && !is_numeric(t.peek())) {
    err_token(token, "decimals in the form of 'x.' are not allowed");
  }

  // Add the decimal part, if it exists.
  while (is_numeric(t.cur_char())) {
    num_literal += t.cur_char();
    t.advance_char();
  }

  token->string_value = num_literal;
  token->type =
      is_float_literal ? TokenType::FloatLiteral : TokenType::IntLiteral;

  return token;
}

// Returns a token for "punctuation". This is a catch-all term for tokens that
// are not symbols or literals.
std::shared_ptr<Token> get_punctuation(ProgramText &t) {
  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;
  token->string_value += t.cur_char();

  switch (t.cur_char()) {
  // All supported "punctuation" characters can be seen here:
  case '(':
    token->type = TokenType::Lparen;
    break;
  case ')':
    token->type = TokenType::Rparen;
    break;
  case '{':
    token->type = TokenType::Lcurl;
    break;
  case '}':
    token->type = TokenType::Rcurl;
    break;
  case '[':
    token->type = TokenType::Lbracket;
    break;
  case ']':
    token->type = TokenType::Rbracket;
    break;
  case ';':
    token->type = TokenType::Semicolon;
    break;
  case ',':
    token->type = TokenType::Comma;
    break;
  default:
    err_token(token, "unrecognized character");
    break;
  }

  t.advance_char();
  return token;
}

// Returns a token for a string literal, like "Hello".
std::shared_ptr<Token> get_string_literal(ProgramText &t) {
  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;
  std::string str_literal;

  str_literal += t.cur_char();
  t.advance_char();

  while (t.cur_char() != '"' && !t.done()) {
    str_literal += t.cur_char();
    t.advance_char();
  }

  // Add in closing quote, if it exists:
  if (t.cur_char() == '"') {
    str_literal += t.cur_char();
    t.advance_char();
  }

  else {
    // No matching quote
    err_token(token, "no matching quote");
  }

  token->type = TokenType::StrLiteral;
  token->string_value = str_literal;

  return token;
}

std::shared_ptr<Token> get_operator(ProgramText &t) {
  // TODO: I think this function needs to get broken up into three different
  // functions: one for binary ops, one for unary ops, and one for assignment.
  // Or maybe not. This should work for now.
  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;

  char cur_char = t.cur_char();
  char next_char = t.peek();
  switch (cur_char) {
  case '+':
    token->type = TokenType::OpPlus;
    t.advance_char();
    break;
  case '-':
    token->type = TokenType::OpMinus;
    t.advance_char();
    break;
  case '*':
    token->type = TokenType::OpTimes;
    t.advance_char();
    break;
  case '/':
    token->type = TokenType::OpDiv;
    t.advance_char();
    break;
  case '%':
    token->type = TokenType::OpRem;
    t.advance_char();
    break;
  case '&': {
    // Two cases here: & (binary AND), or && (logical AND).
    t.advance_char();
    if (next_char == '&') {
      token->type = TokenType::OpBand;
      t.advance_char();
    } else {
      token->type = TokenType::OpAnd;
    }
    break;
  }
  case '|': {
    // Two cases here: | (binary OR), or || (logical OR).
    t.advance_char();
    if (next_char == '|') {
      token->type = TokenType::OpBor;
      t.advance_char();
    } else {
      token->type = TokenType::OpOr;
    }
    break;
  }
  case '<': {
    // Three potential cases: < (less than), <= (less than or equal to), or <>
    // (not equals).
    t.advance_char();
    if (t.peek() == '=') {
      token->type = TokenType::OpLe;
      t.advance_char();
    } else if (t.peek() == '>') {
      token->type = TokenType::OpNe;
      t.advance_char();
    } else {
      token->type = TokenType::OpLt;
    }
    break;
  }
  case '>': {
    // Only two cases: > (greater than) or >= (greater than or equal to).
    t.advance_char();
    if (next_char == '=') {
      token->type = TokenType::OpGe;
      t.advance_char();
    } else {
      token->type = TokenType::OpGt;
    }
    break;
  }
  case '=': {
    if (t.peek() == '=') {
      token->type = TokenType::OpEq;
      t.advance_char();
      t.advance_char();
      break;
    } else {
      err_token(token, "Unrecognized operator");
    }
  }
  case ':': {
    if (t.peek() == '=') {
      token->type = TokenType::Assign;
      t.advance_char();
      t.advance_char();
      break;
    } else {
      err_token(token, "Unrecognized operator");
    }
  }
  default: {
    err_token(token, "Unrecognized operator");
  }
  }

  return token;
}

// Tokenizes the string in a ProgramText into a token list.
std::deque<std::shared_ptr<Token>> tokenize(ProgramText &t) {
  std::deque<std::shared_ptr<Token>> tokens;

  while (!t.done()) {
    // printf("%c %u %u\n", t.cur_char(), t.line_num, t.col_num);

    if (is_numeric(t.cur_char()) ||
        (t.cur_char() == '.' && is_numeric(t.peek()))) {
      tokens.push_back(get_numeric_literal(t));
    }

    // Beginning of a string literal
    else if (t.cur_char() == '"') {
      tokens.push_back(get_string_literal(t));
    }

    // Comments. We'll just skip the rest of the line here.
    else if (t.cur_char() == '#') {
      t.advance_char(); // skip over ;
      while (t.cur_char() != '\r' && t.cur_char() != '\n' && !t.done()) {
        t.advance_char();
      }

      if (t.cur_char() == '\n') {
        t.advance_char();
      }

      else if (t.cur_char() == '\r' && t.peek() == '\n') {
        t.advance_char();
        t.advance_char();
      }
    }

    // Everything else is assumed to be punctuation
    else if (is_punctuation(t.cur_char())) {
      tokens.push_back(get_punctuation(t));
    }

    else if (is_alpha(t.cur_char()) ||
             (t.cur_char() == '_' && is_alpha(t.peek()))) {
      tokens.push_back(get_symbol(t));
    }

    else {
      tokens.push_back(get_operator(t));
    }

    // Skip whitespace characters
    t.skip_whitespace();
  }

  return tokens;
}
