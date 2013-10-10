/*
 * cic_filter.S
 *
 * for dmic soft 
 * by Larry Liu
 */
#include <linux/linkage.h>        
#include <asm/assembler.h>

        .global cic_filter  /* input_addr, output_addr, input_len, para_ptr
                            *    r0          r1           r2        r3
                            * should know that out put size should be 4 * input_len */
/*--------------------------------------------------------------------------------------
 *
 * $ left_shift_bit
 * cic_bit8 is the MACRO to deal with cic bit8 
 * and store each bit8 output result */

    .macro	cic_bit8, left_shift_bit
    /* keep byte at the left 8 bit of r7 */ 
    mov     r0, r9               /* r0 = r9 */
    ldmia   r0!, {r1 - r6}       /* r1 = table, r2 = sig1,...r6 = sig5 */

    mov     r7, r8, lsl #\left_shift_bit    
    
    ldrb    r0, [r1, r7, lsr #24]/* r0 = table[tmp], or tmp */
            
    /* sig 5 */
    add     r6, r6, r3, lsl #3   /* += sig2 << 3 */
    add     r6, r6, r4, lsl #3   /* += sig3 << 3 */
    add     r6, r6, r5, lsl #3   /* += sig4 << 3 */

    add     r0, r0, r2, lsl #3   /* r0 = tmp + sig1 << 3 */

    add     r6, r6, r0           /* += (tmp + sig1 << 3) */
    /* sig 4 */
    add     r5, r5, r4, lsl #3   /* += sig2 << 3 */
    add     r5, r5, r3, lsl #3   /* += sig3 << 3 */
    add     r5, r5, r0           /* += (tmp + sig1 << 3) */
    /* sig 3 */
    add     r4, r4, r3, lsl #3   /* += sig2 << 3 */
    add     r4, r4, r0           /* += (tmp + sig1 << 3) */
    /* sig 2 */
    add     r3, r3, r0           /* += (tmp + sig1 << 3) */
    
    add     r1, r1, #256
    ldrb    r0, [r1, r7, lsr #24]/* r0 = table[tmp + 256], sum */
    add     r2, r2, r0           /* sig 1 = sig1 + sum */
    add     r0, r9, #4           /* r0 = r9 + 4 */

    stmia   r0!, {r2 - r6}       /* store sig1,..., sig5 */
    add     r7, r9, #24          /* r0 = r9 + 24 */
    ldmia   r7!, {r1 - r5}       /* r1 = old1, r2 = old2, ...r5 = old5 */
     
    sub     r6, r6, r1           /* sig5 - old1 */
    sub     r6, r6, r2           /* sig5 - old2 */
    sub     r6, r6, r3           /* sig5 - old3 */
    sub     r6, r6, r4           /* sig5 - old4 */
    sub     r6, r6, r5           /* sig5 - old5 */
    strh    r6, [r12], #2        /* [r12] = sig5 - old1 - .. - old5 */
    
    add     r0, r9, #24

    add     r4, r4, r6           /* old4 += cic_out */
    add     r3, r3, r4           /* old3 += old4 */
    add     r2, r2, r3           /* old2 += old3 */
    add     r1, r1, r2           /* old1 += old2 */
    add     r5, r5, r1           /* old5 += old1 */
    
    stmia   r0!, {r1 - r5}       /* store old1, ...,old5 */
    .endm


/* @ table[] sum   bit sum than  tmp
 * @ size r10    32bit count
 * @ input data addr r11
 * @ output data addr r12
 * @ parameter array pointor r9  
 * array[] = {table_addr, sig1, sig2, sig3, sig4, 
 *            sig5, old1, old2, old3, old4, old5} */
         
cic_filter:
    stmfd   sp!, {r4 - r12, lr}
    
    sub     r10, r2, #1          /* size */    
    mov     r11, r0              /* input addr */
    mov     r12, r1              /* output addr */
    mov     r9, r3
    
cic_bit32:
    ldr     r8, [r11], #0x4      /* r8 = [r11] then r11 += 4 */
    cic_bit8 24
    cic_bit8 16
    cic_bit8 8
    cic_bit8 0
    subs    r10, r10, #1
    bge     cic_bit32
    ldmfd   sp!, {r4 - r12, pc}


        
    
