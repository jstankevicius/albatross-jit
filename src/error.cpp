#include "error.h"

#include <iostream>
#include <sstream>
#include <string>

char const *RED_BEGIN = "\033[1;31m";
char const *RED_END   = "\033[0m";

static std::vector<std::string>
split_string(const std::string &str)
{
        std::vector<std::string> tokens;

        std::stringstream ss(str);
        std::string       token;
        while (std::getline(ss, token, '\n')) {
                tokens.push_back(token);
        }

        return tokens;
}

void
print_err(const std::string &src,
          int                line_num,
          int                col_num,
          const std::string &message)
{
        assert(line_num > 0);
        assert(col_num > 0);

        const int up_limit   = 2;
        const int down_limit = 2;

        auto lines = split_string(src);

        std::cout << RED_BEGIN
                  << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        std::cout << "Error on line " << line_num << ", column " << col_num
                  << ":\n";
        for (int src_line_num = 1; src_line_num <= (int)lines.size();
             ++src_line_num) {
                // Actual index into the array:
                int idx = src_line_num - 1;

                if (line_num - up_limit <= src_line_num
                    && src_line_num <= line_num + down_limit) {
                        std::cout << (src_line_num == line_num ? ">> " : "   ")
                                  << lines[idx] << "\n";
                        if (src_line_num == line_num) {
                                for (int col = 0; col <= col_num + 1; col++) {
                                        std::cout << " ";
                                }
                                std::cout << "^\n";
                        }
                }
        }

        std::cout << "Message: " << message << "\n";
        std::cout << RED_END;
}