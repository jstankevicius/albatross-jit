#include <vector>

struct Frame {
    std::vector<int> stack;

    // Really, a function call frame just needs a stack. On this stack we put
    // arguments, the return address, any registers we are saving, etc. All
    // variables go into our infinite set of registers.

    /**
     * on fn call:
     * 
     * Look at the IR registers that this function will use for local variables.
     * Ignore globals.
     * 
     * 
     */

    /**
     * fun f int(a int) {
     *     var b int := 2;
     *     return a + b + 2;  
     * }
     * 
     * f would use:
     * reg 0 to store 'a'
     * reg 1 to store 'b'
     * reg 2 to store 'a + b'
     * reg 3 to store 'reg 2 + 2'
     * 
     * IR code:
     * caller sets r0 to whatever the value of 'a' is.
     * f:
     *     r1 = 2
     *     r2 = r1 + r0 
     *     r3 = r2 + 2
     *     ret r3
     * 
     * fun factorial(i int) {
     *     if (i == 1) {
     *         return 1;
     *     }
     * 
     *     return i * factorial(i - 1);
     * }
     * 
     * How do we even compile this:
     * r0 is used for 'i'
     * r1 would be used for (i - 1);
     * r2 for factorial(i - 1)
     * rr for i * factorial(i - 1)
     * 
     * factorial:
     *     cmp r0, $1
     *     jnz L1
     *     mv rr, $1
     *     ret
     * L1:
     *     r1 = r0 - 1
     *     <SAVE R0, R1, R2>
     *     r0 = r1 // load param for call to factorial
     *     call factorial
     *     <RESTORE R0, R1, R2>
     *     r2 = rr // rr would have been populated by previous call
     *     rr = rr * r0
     *     ret
     *     
     * a = b + c + d
     * foo = a + 2
     * 
     * a = (b + c) + d
     * r0 =   r1   + r2
     * r1 =   r0   + 2
     * 
     * save r0
     * r0 <- r1
     * call factorial
     * r2 
    */
};