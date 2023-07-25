#pragma once

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "token.h"

// A ProgramText bundles together a stream (a program represented as a string)
// and a current position within that string.
struct ProgramText {
  unsigned int idx = 0;
  unsigned int line_num = 1;
  unsigned int col_num = 1;

  std::string stream;

  ProgramText(std::string &stream) : stream(stream) {}

  bool done();
  char cur_char();
  char peek();
  void advance_char();
  char next();
  void skip_whitespace();
};

std::deque<std::shared_ptr<Token>> tokenize(ProgramText &t);
