#include <deque>
#include <fstream>
#include <iostream>
#include <string>

#include "lexer.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    // This should just switch to interactive mode
    std::cerr << "Error: no input file" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ifstream file;
  file.open(argv[1]);

  if (!file.is_open()) {
    perror("Error: open()");
    exit(EXIT_FAILURE);
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      (std::istreambuf_iterator<char>()));

  ProgramText handle(content);

  auto tokens = tokenize(handle);
}
