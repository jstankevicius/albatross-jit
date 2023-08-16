#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compiler_stages.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "symres.h"
#include "typecheck.h"

int main(int argc, char *argv[])
{
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

        try {
#ifdef COMPILE_STAGE_LEXER
                ProgramText text(content);

                auto tokens = tokenize(text);

#ifdef COMPILE_STAGE_PARSER
                auto stmts = parse_stmts(tokens);

#ifdef COMPILE_STAGE_SYMBOL_RESOLVER
                SymbolTable<VarInfo> vars;
                SymbolTable<FunInfo> functions;

                functions.enter_scope();
                vars.enter_scope();

                resolve_stmts(stmts, vars, functions);
#ifdef COMPILE_STAGE_TYPE_CHECKER
                typecheck_stmts(stmts);
#endif
#endif
#endif
#endif
        } catch (AlbatrossError &e) {
                print_err(content, e.line_num(), e.col_num(), e.what());
                exit(e.exit_code());
        }
}
// #include "ir.h"
// #include <iostream>

// struct Timer
// {
//     Timer() { get(&start_tp); }
//     double elapsed() const {
//         struct timespec end_tp;
//         get(&end_tp);
//         return (end_tp.tv_sec - start_tp.tv_sec) +
//                (1E-9 * end_tp.tv_nsec - 1E-9 * start_tp.tv_nsec);
//     }
//   private:
//     static void get(struct timespec* p_tp) {
//         if (clock_gettime(CLOCK_REALTIME, p_tp) != 0)
//         {
//             std::cerr << "clock_gettime() error\n";
//             exit(EXIT_FAILURE);
//         }
//     }
//     struct timespec start_tp;
// };

// int main() {
//   std::cout << "Instruction size: " << sizeof(Instruction) << "bytes\n";

//   std::vector<Instruction> ins;
//   for (int i=0; i < 10000000; i++) {
//     Instruction in;
//     in.type=InstructionType::Add;
//     ins.push_back(in);
//   }

//   int z = 0;
//   Timer t;
//   for (unsigned int i = 0; i < ins.size(); i++) {
//     Instruction in = ins[i];
//     InstructionType type = in.type;

//     uint16_t arg1 = in.data.three_args.arg1;
//     uint16_t arg2 = in.data.three_args.arg2;
//     uint16_t arg3 = in.data.three_args.arg3;
//     ArgType arg1_type = in.data.three_args.arg1_type;
//     ArgType arg2_type = in.data.three_args.arg2_type;
//     ArgType arg3_type = in.data.three_args.arg3_type;

//     z += arg1 + arg2 + arg3;
//   }
//   std::cout << z << "\n";
//   std::cout << t.elapsed() << "\n";
// }