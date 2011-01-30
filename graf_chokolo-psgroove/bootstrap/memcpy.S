
#include <ppc_asm.h>

_GLOBAL(memcpy)
	cmpdi %r5, 0
	beq 2f
	mtctr %r5
	li %r5, 0
1:
	lbzx %r6, %r4, %r5
	stbx %r6, %r3, %r5
	addi %r5, %r5, 1
	bdnz 1b
2:
	blr
