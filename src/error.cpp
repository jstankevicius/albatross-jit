#include "error.h"

#include <string.h>

#include <iostream>

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
