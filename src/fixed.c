#include "fixed.h"

#if 0
/* fast 16.16 division ripped from http://me.henri.net/fp-div.html */
int32_t x16div(register int32_t numerator, register int32_t denominator)
{
    register int32_t quotient;
    asm("num     .req %[numerator]      @ Map Register Equates\n\t"
        "den     .req %[denominator]\n\t"
        "mod     .req r2\n\t"
        "cnt     .req r3\n\t"
        "quo     .req r4\n\t"
        "sign    .req r12\n\t"
        /* set sign and ensure numerator and denominator are positive */
        "cmp den, #0                    @ exceptioin if den == zero\n\t"
        "beq .div0\n\t"
        "eor sign, num, den             @ sign = num ^ den\n\t"
        "rsbmi den, den, #0             @ den = -den if den < 0\n\t"
        "subs mod, den, #1              @ mod = den - 1\n\t"
        "beq .div1                      @ return if den == 1\n\t"
        "movs cnt, num                  @ num = -num if num < 0\n\t"
        "rsbmi num, num, #0\n\t"
        /* skip if deniminator >= numerator */
        "movs cnt, num, lsr #16         @ return if den >= num << 16\n\t"
        "bne .cont\n\t"
        "cmp den, num, lsl #16\n\t"
        "bhs .numLeDen\n\t"
    "\n.cont:\n\t"
        /* test if denominator is a power of two */
        "tst den, mod                   @ if(den & (den - 1) == 0)\n\t"
        "beq .powerOf2                  @ den is power of 2\n\t"
        /* count leading zeros */
        "stmfd sp!, {r4}                @ push r4 (quo) onto the stack\n\t"
        "mov cnt, #28                   @ count difference in leading zeros\n\t"
        "mov mod, num, lsr #4           @ between num and den\n\t"
        "cmp den, mod, lsr #12; subls cnt, cnt, #16; movls mod, mod, lsr #16\n\t"
        "cmp den, mod, lsr #4 ; subls cnt, cnt, #8 ; movls mod, mod, lsr #8\n\t"
        "cmp den, mod         ; subls cnt, cnt, #4 ; movls mod, mod, lsr #4\n\t"
        /* shift numerator left by cnt bits */
        "mov num, num, lsl cnt          @ mod:num = num << cnt\n\t"
        "mov quo, #0\n\t"
        "rsb den, den, #0               @ negate den for divide loop\n\t"
        /* skip cnt iterations in the divide loop */
        "adds num, num, num             @ start: num = mod:num / den\n\t"
        "add pc, pc, cnt, lsl #4        @ skip cnt x 4 x 4 iterations\n\t"
        "nop                            @ nop instruction takes care of pipeline\n\t"
        /* inner loop unrolled x 48 */
        ".rept 47                       @ inner loop x 48\n\t"
        "    adcs mod, den, mod, lsl #1\n\t"
        "    subcc mod, mod, den\n\t"
        "    adc quo, quo, quo\n\t"
        "    adds num, num, num\n\t"
        ".endr\n\t"
        "adcs mod, den, mod, lsl #1\n\t"
        "subcc mod, mod, den\n\t"
        "adc quo, quo, quo\n\t"
        /* negate quotient if signed */
        "cmp sign, #0                   @ negate quotient if sign < 0\n\t"
        "mov num, quo\n\t"
        "rsbmi num, num, #0\n\t"
        "ldmfd sp!, {r4}                @ pop r4 (quo) off the stack\n\t"
        "mov pc, lr                     @return\n\t"
        /* divide by zero handler */
    "\n.div0:\n\t"
        "mov num, #0\n\t"
        "mov pc, lr                     @return\n\t"
        /* divide by one handler */
    "\n.div1:\n\t"
        "cmp sign, #0\n\t"
        "mov num, num, asl #16\n\t"
        "rsbmi num, num, #0\n\t"
        "mov pc, lr                     @return\n\t"
        /* numerator less than or equal to denominator handler */
    "\n.numLeDen:\n\t"
        "mov num, #0                    @ quotient = 0 if num < den\n\t"
        "moveq num, sign, asr #31       @ negate quotient if sign < 0\n\t"
        "orreq num, num, #1             @ quotient = 1 if num == den\n\t"
        "mov pc, lr                     @return\n\t"
        /* power of two handler */
    "\n.powerOf2:\n\t"
        "mov cnt, #0\n\t"
        "cmp den, #(1 << 16); movhs cnt, #16    ; movhs den, den, lsr #16\n\t"
        "cmp den, #(1 << 8) ; addhs cnt, cnt, #8; movhs den, den, lsr #8\n\t"
        "cmp den, #(1 << 4) ; addhs cnt, cnt, #4; movhs den, den, lsr #4\n\t"
        "cmp den, #(1 << 2) ; addhi cnt, cnt, #3; addls cnt, cnt, den, lsr #1\n\t"
        "rsb mod, cnt, #32\n\t"
        "mov den, num, lsr #16          @ den:num = num << 16\n\t"
        "mov num, num, lsl #16\n\t"
        "mov num, num, lsr cnt          @ num = num >> cnt | den << mod\n\t"
        "orr num, num, den, lsl mod\n\t"
        "cmp sign, #0\n\t"
        "rsbmi num, num, #0             @ negate quotient if sign < 0"
        /* output registers */
        : [quotient] "=r" (quotient)
        /* input registers */
        : [numerator] "0" (numerator), [denominator] "r" (denominator)
        /* clobbered registers */
        : "r2" /* mod */, "r3" /* cnt */, "r12" /* sign */);
    return quotient;
}
#endif
