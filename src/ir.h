#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class InstructionType : unsigned char {
        Halt = 0,
        Mov,

        Add,
        Sub,
        Neg,
        Mul,
        Div,

        Jmp,
        Jz,
        Jnz,
        Je,
        Ret,

        Or,
        And,
        Xor,

        Ne, // ne R1, R2 == xor R1, R2; bzero R1
        Eq, // eq R1, R2 == xor R1, R2; bzero R1
        Not,

        Gt,
        Ge,
        Lt,
        Le,
        Rem
};

enum class ArgType : unsigned char { Invalid, Reg, Imm, Addr };

// We could also do #pragma pack(push, 1).
struct Instruction {
        InstructionType type;

        union {
                struct {
                        uint16_t arg1;
                        uint16_t arg2;
                        uint16_t arg3;
                        ArgType  arg1_type;
                        ArgType  arg2_type;
                        ArgType  arg3_type;
                } three_args;
        } data;
};

/*

auto block = create_block(); // block gets assigned a label
          d  s  s
block.add(0, 1, 2)
R0 = 1 + 2 <- we can just compute this value and replace it with an R0 = 3

struct Node {
  std::vector<Instruction> instructions;


}
*/

struct Node {
        std::vector<Instruction> instructions;

        // Having a "next" would implicitly insert a jump instruction. We could also
        // just have a jump (conditional or not) as the last instruction in the
        // block.
        // Maybe just leave the last instruction as a jmp/jnz/je/jal but have
        // optional shared_ptrs to two nodes? Like this:
        std::optional<std::shared_ptr<Node>> branch_1;
        std::optional<std::shared_ptr<Node>> branch_2;

        // So the jump instructions would just use the branches like this:
        /*
jmp: branch_1 (unconditional)
jnz: branch_1 (true) branch_2 (false)
jz:  branch_1 (true) branch_2 (false)
je:  branch_1 (true) branch_2 (false)
jal: branch_1 ?
ret: none.
*/
};