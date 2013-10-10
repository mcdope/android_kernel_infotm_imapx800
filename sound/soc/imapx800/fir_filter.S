/*
 * fir filter.S
 *
 * for dmic soft
 * by Larry Liu
 */
#include <linux/linkage.h>
#include <asm/assembler.h>

        .global fir_filter /* *input_addr, *output_addr, size, *fir_16 
                            *   r0          r1         r2        r3    */
        .global fir_filter1 /* *input_addr, *output_addr, size, *fir_16 
                             *   r0          r1         r2        r3    */
                           
/*--------------------------------------------------------------------------------------
 *
 * @ input data  : r0  uint16_t
 * @ output data : r1  uint16_t  here we should know that output size is half of input size as our default decimation is 2
 * @ size : r2  the size of input data by uint16_t         
 * @ fir_16 : r3  it stores parameters used in the fir
 *                it defines in the .h, and the assembly is strictly to the define of origin struct
 *                if you wanna change some member of struct, please change fir_filter code as the same time
 *                or it may cause some errors   
 *
 * @ fir`s decimation is 2, and all the data type is short, number of coefficients is 21
 * k = 21         
 */
fir_filter:
    stmfd  sp!, {r4 - r12, lr}

    lsr r2, r2, #1            /* r2 = size << 1 */
    ldr r4, [r3], #4          /* r4 = *h, r3 = fir_16->buf */
    add r5, r3, #40           /* offset 20 * 2 */
    mov r6, r0        
    
    ldmia r6!, {r7 - r12}     /* buf[k-1 ~ 2k] = input_data */
    stmia r5!, {r7 - r12}
    ldmia r6!, {r7 - r11}
    mov   r12, r3
    stmia r5!, {r7 - r11}
    stmfd sp!, {r0, r2 - r4}
    mov   r0, r12
    mov   r2, #10
    mov   r3, r4 
    bl    fir  
    ldmfd sp!, {r0, r2 - r4}
    add   r5, r0, r2, lsl #2
    sub   r5, r5, #40
    mov   r11, r3
    ldmia r5!, {r6 - r10}
    stmia r11!, {r6 - r10}
    sub   r2, r2, #10
    ldmia r5!, {r6 - r10}
    mov   r3, r4
    stmia r11!, {r6 - r10}
    bl    fir 

    ldmfd  sp!, {r4 - r12, pc}
    
/* @ r0, *input
 * @ r1, *output
 * @ r2, size
 * @ r3, *h
 *
 * this function is for k = 21*/
fir:
    str     lr, [sp, #-4]!
sub_fir:
    ldmia   r0!, {r6 - r9}
    mov     r5, #0
    ldmia   r3!, {r10 - r12, lr}
    smlad   r4, r6, r10, r5
    smlad   r5, r7, r11, r4
    smlad   r4, r8, r12, r5
    smlad   r5, r9, lr,  r4
    ldmia   r0!, {r6 - r9}
    ldmia   r3!, {r10 - r12, lr}
    smlad   r4, r6, r10, r5
    smlad   r5, r7, r11, r4
    smlad   r4, r8, r12, r5
    smlad   r5, r9, lr,  r4
    ldmia   r0!, {r6 - r8}
    ldmia   r3!, {r9 - r11}
    sub     r0, r0, #40
    smlad   r4, r6, r9, r5
    smlad   r5, r7, r10, r4
    smlad   r4, r8, r11, r5
    sub     r3, r3, #44
    lsr     r4, r4, #15       
    strh    r4, [r1], #2
    subs    r2, r2, #1
    bne     sub_fir
    pop     {pc}



fir_filter1:
    stmfd  sp!, {r4 - r12, lr}

    lsr r2, r2, #1            /* r2 = size << 1 */
    ldr r4, [r3], #4          /* r4 = *h, r3 = fir_16->buf */
    add r5, r3, #40           /* offset 20 * 2 */
    mov r6, r0        
    
    ldmia r6!, {r7 - r12}     /* buf[k-1 ~ 2k] = input_data */
    stmia r5!, {r7 - r12}
    ldmia r6!, {r7 - r11}
    mov   r12, r3
    stmia r5!, {r7 - r11}
    stmfd sp!, {r0, r2 - r4}
    mov   r0, r12
    mov   r2, #10
    mov   r3, r4 
    bl    fir1  
    ldmfd sp!, {r0, r2 - r4}
    add   r5, r0, r2, lsl #2
    sub   r5, r5, #40
    mov   r11, r3
    ldmia r5!, {r6 - r10}
    stmia r11!, {r6 - r10}
    sub   r2, r2, #10
    ldmia r5!, {r6 - r10}
    mov   r3, r4
    stmia r11!, {r6 - r10}
    bl    fir1 

    ldmfd  sp!, {r4 - r12, pc}
    
/* @ r0, *input
 * @ r1, *output
 * @ r2, size
 * @ r3, *h
 *
 * this function is for k = 21*/
fir1:
    str     lr, [sp, #-4]!
sub_fir1:
    ldmia   r0!, {r6 - r9}
    mov     r5, #0
    ldmia   r3!, {r10 - r12, lr}
    smlad   r4, r6, r10, r5
    smlad   r5, r7, r11, r4
    smlad   r4, r8, r12, r5
    smlad   r5, r9, lr,  r4
    ldmia   r0!, {r6 - r9}
    ldmia   r3!, {r10 - r12, lr}
    smlad   r4, r6, r10, r5
    smlad   r5, r7, r11, r4
    smlad   r4, r8, r12, r5
    smlad   r5, r9, lr,  r4
    ldmia   r0!, {r6 - r8}
    ldmia   r3!, {r9 - r11}
    sub     r0, r0, #40
    smlad   r4, r6, r9, r5
    smlad   r5, r7, r10, r4
    smlad   r4, r8, r11, r5
    sub     r3, r3, #44
    lsr     r4, r4, #15       
    strh    r4, [r1], #2
    strh    r4, [r1], #2
    subs    r2, r2, #1
    bne     sub_fir1
    pop     {pc}
