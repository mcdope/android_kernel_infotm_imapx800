#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/memory.h>

	.text

ENTRY(imapx800_cpu_save)

	stmfd	sp!, { r3 - r12, lr }
	ldr	r3, =resume_with_mmu
	bl	cpu_suspend

	ldr	r0, =pm_cpu_sleep
	ldr	r0, [ r0 ]
	mov	pc, r0

resume_with_mmu:
	ldmfd	sp!, { r3 - r12, pc }

	.ltorg

	.word	0x2bedf00d

ENTRY(imapx800_cpu_resume)
	b	cpu_resume


