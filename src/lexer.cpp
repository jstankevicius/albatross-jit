#include "lexer.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "compiler_stages.h"
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

char ProgramText::next() {
  char c = cur_char();
  advance_char();
  return c;
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
    keyword_map["var"] = TokenType::KeywordVar;
    keyword_map["if"] = TokenType::KeywordIf;
    keyword_map["else"] = TokenType::KeywordElse;
    keyword_map["while"] = TokenType::KeywordWhile;
    keyword_map["return"] = TokenType::KeywordReturn;
    keyword_map["otherwise"] = TokenType::KeywordOtherwise;
    keyword_map["repeat"] = TokenType::KeywordRepeat;
    keyword_map["fun"] = TokenType::KeywordFun;
    keyword_map["int"] = TokenType::TypeName;
    keyword_map["string"] = TokenType::TypeName;
    keyword_map["char"] = TokenType::TypeName;
    keyword_map["void"] = TokenType::TypeName;
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
         (is_alphanumeric(t.cur_char()) || t.cur_char() == '_')) {
    str += t.next();
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
  // TODO: Oh my god fix this
  auto token = std::make_shared<Token>();
  token->col_num = t.col_num;
  token->line_num = t.line_num;
  token->stream = &t.stream;
  token->type = TokenType::IntLiteral;
  std::string num_literal;

  // Check the type of integer:
  // 0x -> hex
  // 0  -> octal
  // everything else -> decimal
  int base = -1;

  if (t.cur_char() == '0') {
    if (t.peek() == 'x') {
      // Hexadecimal
      base = 16;
      t.advance_char();
      t.advance_char();
    }
    else {
      // Octal
      base = 8;
    }
  } else {
    // Decimal
    base = 10;
  }

  if (t.cur_char() == '_') {
    throw AlbatrossError("Illegal int literal " + num_literal, t.line_num, t.col_num, EXIT_LEXER_FAILURE);
  }

  while (is_alphanumeric(t.cur_char())) {
    // Check, depending on the base, for illegal digits:
    char c = std::toupper(t.cur_char());
    if ((base == 8 && !('0' <= c && c <= '7'))
    || (base == 10 && !('0' <= c && c <= '9'))
    || (base == 16 && !(('0' <= c && c <= '9') || ('A' <= c && c <= 'F')))) {
      throw AlbatrossError("Illegal digit for int of base " + std::to_string(base), t.line_num, t.col_num, EXIT_LEXER_FAILURE);
    }

    num_literal += t.next();

    // Skip underscores
    while (t.cur_char() == '_') {
      t.advance_char();
    }
  }

  try { 
    num_literal = std::to_string(std::stoi(num_literal, 0, base));
  }

  catch (std::invalid_argument &e) {
    throw AlbatrossError("Illegal int literal " + num_literal, t.line_num, t.col_num, EXIT_LEXER_FAILURE);
  }
  catch (std::out_of_range &e) {
    throw AlbatrossError("Int " + num_literal + " is out of range", t.line_num, t.col_num, EXIT_LEXER_FAILURE);
  }
  token->string_value = num_literal;

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
    throw AlbatrossError("unrecognized character", t.line_num, t.col_num,
                         EXIT_LEXER_FAILURE);
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

  // str_literal += t.cur_char();
  // Skip opening quote
  t.advance_char();

  while (t.cur_char() != '"' && !t.done()) {
    if (t.cur_char() == '\\') {
      if (t.peek() == 'n') {
        str_literal += '\n';
        t.advance_char();
        t.advance_char();
        continue;
      }

      else if (t.peek() == 't') {
        str_literal += '\t';
        t.advance_char();
        t.advance_char();
        continue;
      }

      else if (t.peek() == '\\') {
        str_literal += '\\';
        t.advance_char();
        t.advance_char();
        continue;
      }

      else if (t.peek() == '\"') {
        str_literal += '\"';
        t.advance_char();
        t.advance_char();
        continue;
      }

      else {
        throw AlbatrossError("Invalid escape sequence", t.line_num, t.col_num,
                             EXIT_LEXER_FAILURE);
      }
    } else if (t.cur_char() == '\n') {
      throw AlbatrossError("no matching quote", t.line_num, t.col_num,
                           EXIT_LEXER_FAILURE);
    }

    str_literal += t.next();
  }

  // Add in closing quote, if it exists:
  if (t.cur_char() == '"') {
    // str_literal += t.cur_char();
    t.advance_char();
  }

  else {
    // No matching quote
    throw AlbatrossError("no matching quote", t.line_num, t.col_num,
                         EXIT_LEXER_FAILURE);
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
    token->string_value += t.next();
    break;
  case '-':
    token->type = TokenType::OpMinus;
    token->string_value += t.next();
    break;
  case '*':
    token->type = TokenType::OpTimes;
    token->string_value += t.next();
    break;
  case '/':
    token->type = TokenType::OpDiv;
    token->string_value += t.next();
    break;
  case '%':
    token->type = TokenType::OpRem;
    token->string_value += t.next();
    break;
  case '!': {
    token->type = TokenType::OpNot;
    token->string_value += t.next();
    break;
  }
  case '&': {
    // Two cases here: & (binary AND), or && (logical AND).
    token->string_value += t.next();
    if (next_char == '&') {
      token->type = TokenType::OpAnd;
      token->string_value += t.next();
    } else {
      token->type = TokenType::OpBand;
    }
    break;
  }
  case '|': {
    // Two cases here: | (binary OR), or || (logical OR).
    token->string_value += t.next();
    if (next_char == '|') {
      token->type = TokenType::OpOr;
      token->string_value += t.next();
    } else {
      token->type = TokenType::OpBor;
    }
    break;
  }
  case '^': {
    token->type = TokenType::OpXor;
    token->string_value += t.next();
    break;
  }
  case '<': {
    // Three potential cases: < (less than), <= (less than or equal to), or <>
    // (not equals).
    token->string_value += t.next();
    if (next_char == '=') {
      token->type = TokenType::OpLe;
      token->string_value += t.next();
    } else if (next_char == '>') {
      token->type = TokenType::OpNe;
      token->string_value += t.next();
    } else {
      token->type = TokenType::OpLt;
    }
    break;
  }
  case '>': {
    // Only two cases: > (greater than) or >= (greater than or equal to).
    token->string_value += t.next();
    if (next_char == '=') {
      token->type = TokenType::OpGe;
      token->string_value += t.next();
    } else {
      token->type = TokenType::OpGt;
    }
    break;
  }
  case '=': {
    if (t.peek() == '=') {
      token->type = TokenType::OpEq;
      token->string_value += t.next();
      token->string_value += t.next();
      break;
    } else {
      throw AlbatrossError("unrecognized character", t.line_num, t.col_num,
                           EXIT_LEXER_FAILURE);
    }
  }
  case ':': {
    if (t.peek() == '=') {
      token->type = TokenType::Assign;
      token->string_value += t.next();
      token->string_value += t.next();
      break;
    } else {
      throw AlbatrossError("unrecognized character", t.line_num, t.col_num,
                           EXIT_LEXER_FAILURE);
    }
  }
  default: {
    throw AlbatrossError("unrecognized character", t.line_num, t.col_num,
                         EXIT_LEXER_FAILURE);
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

    else if (is_alpha(t.cur_char())) {
      tokens.push_back(get_symbol(t));
    }

    else {
      tokens.push_back(get_operator(t));
    }

    // Skip whitespace characters
    t.skip_whitespace();
  }

#ifdef COMPILE_STAGE_LEXER
#ifndef COMPILE_STAGE_PARSER
#ifndef COMPILE_STAGE_SYMBOL_RESOLVER
#ifndef COMPILE_STAGE_TYPE_CHECKER
  // This section of the code is only compiled when running compatibility tests
  // for the Albatross lexer. The old tests require the lexer to produce
  // specific output for each token, which normally consists of the column
  // number, line number, and token type. Certain tokens also produce extra
  // output depending on their contents.
  //
  // Some choices of "correct" output are a bit strange to me, but we abide by
  // the requirements anyway.
  // One example is tests/lexer-tests/06-vars-pass2.albatross:
  //
  //    int name
  //    char of
  //    string many
  //    int[] variables
  //
  // This is required to produce the following output:
  //
  //    1 1 TYPE int
  //    5 1 NAME name TYPE int
  //    1 2 TYPE char
  //    6 2 NAME of TYPE char
  //    1 3 TYPE string
  //    8 3 NAME many TYPE string
  //    1 4 TYPE int
  //    4 4 [
  //    5 4 ]
  //    7 4 NAME variables TYPE int
  //
  // Variable declarations do not work this way in Albatross (a variable is
  // declared as 'var x int := 1;', or something similar). But no matter; this
  // is a 'passing' test case for the lexer, and there are plenty of passing
  // test cases that are not valid Albatross programs. But if passing cases are
  // not necessarily valid Albatross programs (and therefore do not have any
  // semantic meaning under the Albatross specification), why embed semantic
  // meaning in a test case for the *lexer*? If `name` was really of type `int`,
  // it wouldn't have been declared as `int name`, but instead
  // `var name int := 0;`, or at least `name int`.
  //
  // An even stranger case is tests/lexer-tests/06-vars-pass3.albatross. I
  // present only a section of the program:
  //
  //
  //    void function(char argument) {
  //      string ret;
  //      if ( argument == 10)
  //        return 10;
  //      else
  //        return function(argument-1);
  //    }
  //
  // Confusingly, this section is expected to produce the following output:
  //
  //    1 1 TYPE void
  //    6 1 NAME function TYPE void
  //    14 1 (
  //    15 1 TYPE char
  //    20 1 NAME argument TYPE char
  //    28 1 )
  //    30 1 {
  //    3 2 TYPE string
  //    10 2 NAME ret TYPE string
  //    13 2 SEMICOLON
  //    3 3 IF
  //    6 3 (
  //    8 3 NAME argument TYPE string
  //    17 3 EQ
  //    20 3 INT 10
  //    22 3 )
  //    5 4 RETURN
  //    12 4 INT 10
  //    14 4 SEMICOLON
  //    3 5 ELSE
  //    5 6 RETURN
  //    12 6 NAME function TYPE string
  //    20 6 (
  //    21 6 NAME argument TYPE string
  //    29 6 MINUS
  //    30 6 INT 1
  //    31 6 )
  //    32 6 SEMICOLON
  //    1 7 }
  //
  // How does seeing a typename (like `string`) somehow cascade it to every
  // named variable (or function) that follows? Here, the `string` in
  // `string ret;` ends up attaching itself to `function` and `argument`,
  // neither of which have the type `string`. This isn't a valid Albatross
  // program (in fact, the C parser would accept this, but not the Albatross
  // parser), but the "correct" output again attaches some kind of semantic
  // meaning to the program anyway.
  // Let's beat the dead horse one more time with
  // tests/lexer-tests/08-strings/pass1.albatross:
  //
  //     string var := "strings constants look like this";
  //     string str := "this is a longer string literal constant";
  //     string sml := "and a small one";
  //
  // Expected output:
  //
  //     1 1 TYPE string
  //     8 1 NAME var TYPE string
  //     12 1 ASSIGN
  //     15 1 STRING 32 strings constants look like this
  //     49 1 SEMICOLON
  //     ... some more lines ...
  //
  // Despite the fact that pretty much every keyword simply gets printed as the
  // capitalized version of that keyword (for example, `repeat` as `REPEAT`),
  // we ignore this convention completely here and instead treat `var` as a
  // name. Oh well.
  //
  // I should quit poking at these test cases though, because there is a very
  // simple explanation for this: When they were created, Albatross's syntax
  // wasn't finalized yet. It was probably assumed that things like `var`
  // wouldn't exist, types would be declared C-style, and functions wouldn't
  // require a `func` keyword. I still don't agree with the idea of trying to
  // deduce the type of the variable in the lexer, but I can forgive the
  // fairly inconsistent treatment of keywords. Those tests (and the following
  // section of code) are there because one of the goals of this project was to
  // pass every test case in Albatross. If I had not chosen that goal, I could
  // have simply implemented a language with my own syntax, or simply gone back
  // to working on my Lisp interpreter. Had I chosen the latter, I probably
  // wouldn't have had to reimplement the entire frontend for the language. But
  // I'm glad I did! It feels a lot more satisfying to pass test cases you
  // struggled through in university, even if you don't agree with them. :)

  std::string type_str = "";

  for (auto token : tokens) {
    std::cout << token->col_num << " " << token->line_num << " ";

    switch (token->type) {
    case TokenType::KeywordVar:
    case TokenType::Identifier: {
      if (type_str.size() > 0) {
        std::cout << "NAME " << token->string_value << " TYPE " << type_str;
      } else {
        std::cout << "NAME " << token->string_value;
      }
      break;
    }
    case TokenType::IntLiteral:
      std::cout << "INT " << token->string_value;
      break;
    case TokenType::Semicolon:
      std::cout << "SEMICOLON";
      break;
    case TokenType::Comma:
      std::cout << "COMMA";
      break;
    case TokenType::Assign:
      std::cout << "ASSIGN";
      break;
    case TokenType::TypeName: {
      type_str = token->string_value;
      std::cout << "TYPE " << token->string_value;
      break;
    }
    case TokenType::StrLiteral: {
      std::cout << "STRING " << token->string_value.size() << " "
                << token->string_value;
      break;
    }

    case TokenType::OpOr:
      std::cout << "OR";
      break;
    case TokenType::OpAnd:
      std::cout << "AND";
      break;
    case TokenType::OpBor:
      std::cout << "BOR";
      break;
    case TokenType::OpXor:
      std::cout << "XOR";
      break;
    case TokenType::OpBand:
      std::cout << "BAND";
      break;
    case TokenType::OpNe:
      std::cout << "NE";
      break;
    case TokenType::OpEq:
      std::cout << "EQ";
      break;
    case TokenType::OpGt:
      std::cout << "GT";
      break;
    case TokenType::OpGe:
      std::cout << "GE";
      break;
    case TokenType::OpLt:
      std::cout << "LT";
      break;
    case TokenType::OpLe:
      std::cout << "LE";
      break;
    case TokenType::OpPlus:
      std::cout << "PLUS";
      break;
    case TokenType::OpMinus:
      std::cout << "MINUS";
      break;
    case TokenType::OpTimes:
      std::cout << "MUL";
      break;
    case TokenType::OpDiv:
      std::cout << "DIV";
      break;
    case TokenType::OpRem:
      std::cout << "REM";
      break;
    case TokenType::OpNot:
      std::cout << "NOT";
      break;

    // Everything below here just gets converted to uppercase:
    case TokenType::Lparen:
    case TokenType::Rparen:
    case TokenType::Lcurl:
    case TokenType::Rcurl:
    case TokenType::Lbracket:
    case TokenType::Rbracket:
    case TokenType::KeywordIf:
    case TokenType::KeywordElse:
    case TokenType::KeywordWhile:
    case TokenType::KeywordReturn:
    case TokenType::KeywordOtherwise:
    case TokenType::KeywordRepeat:
    case TokenType::KeywordFun: {
      int len = token->string_value.size();
      for (int i = 0; i < len; i++) {
        std::cout << (char)std::toupper(token->string_value[i]);
      }
      break;
    }
    default:
      std::cout << "bad\n";
      exit(-1);
    }
    std::cout << "\n";
  }
#endif
#endif
#endif
#endif

  std::shared_ptr<Token> eof_token = std::make_shared<Token>();
  eof_token->line_num = t.line_num;
  eof_token->col_num = t.col_num;
  eof_token->stream = &t.stream;
  eof_token->type = TokenType::Eof;
  tokens.push_back(eof_token);

  return tokens;
}
