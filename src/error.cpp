#include "error.h"

#include <iostream>
#include <sstream>
#include <string>

char const *RED_BEGIN = "\033[1;31m";
char const *RED_END = "\033[0m";

static std::vector<std::string> split_string(const std::string &str) {
  std::vector<std::string> tokens;

  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, '\n')) {
    tokens.push_back(token);
  }

  return tokens;
}

void print_err(const std::string &src, int line_num,
               const std::string &message) {

  const int up_limit = 2;
  const int down_limit = 2;

  auto lines = split_string(src);

  std::cout << RED_BEGIN << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
  std::cout << "Error on line " << line_num << ":\n";
  for (int src_line_num = 1; src_line_num <= (int)lines.size();
       ++src_line_num) {

    // Actual index into the array:
    int idx = src_line_num - 1;

    if (line_num - up_limit <= src_line_num &&
        src_line_num <= line_num + down_limit) {
      std::cout << (src_line_num == line_num ? ">> " : "   ") << lines[idx]
                << "\n";
    }
  }

  std::cout << "Message: " << message << "\n";
  std::cout << RED_END;
}


void err_token(std::shared_ptr<Token> token, std::string message) {
  // TODO: Avoid pointer deref here
  std::string stream = *(token->stream);
  std::cout << "ERROR: line " << token->line_num << ", column "
            << token->col_num << std::endl;

  size_t i = 0;
  for (int line = 1; line < token->line_num; line++) {
    while (stream[i] != '\n' && stream[i] != '\r') {
      i++;
    }

    // Handle \r\n. If peeking one character ahead would put us outside the
    // stream, just return -1.
    if (stream[i] == 'r' &&
        (i + 1 < stream.length() ? stream[i + 1] : -1) == '\n') {
      i++;
    }

    // We've ended on top of a \n, so move by one more char.
    i++;
  }

  // Print this entire current line:
  std::string line;

  while (stream[i] != '\n' && stream[i] != '\r' && i < stream.length()) {
    line += stream[i];
    i++;
  }

  std::cout << line << std::endl;

  // Print the message under the line:
  for (int i = 1; i < token->col_num; i++) {
    std::cout << " ";
  }

  // Underline the offending token
  for (int i = 0;
       i < std::max(1, static_cast<int>(token->string_value.length())); i++) {
    std::cout << "^";
  }

  std::cout << " " << message << std::endl;

  exit(-1);
}
