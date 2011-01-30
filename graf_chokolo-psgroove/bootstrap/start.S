
.org 0

.globl _start
_start:

	stdu %r1, -0x40(%r1)
	mflr %r0
	std %r0, 0x50(%r1)
	std %r2, 0x38(%r1)

	bl 1f

1:

	mflr %r3
	lis %r4, 1b@ha
	addi %r4, %r4, 1b@l
	subf. %r3, %r4, %r3
	beq 3f

	lis %r4, __toc_start@ha
	addi %r4, %r4, __toc_start@l
	lis %r5, __toc_end@ha
	addi %r5, %r5, __toc_end@l
	subf. %r5, %r4, %r5
	beq 3f
	srdi. %r5, %r5, 3
	mtctr %r5
	add %r4, %r4, %r3

2:

	ld %r5, 0(%r4)
	add %r5, %r5, %r3
	std %r5, 0(%r4)
	addi %r4, %r4, 8
	bdnz 2b

3:

	lis %r4, __toc_start@ha
	addi %r4, %r4, __toc_start@l
	add %r2, %r4, %r3
	li %r4, 0
	ori %r4, %r4, 0x8000
	add %r2, %r2, %r4

	bl main

	ld %r2, 0x38(%r1)
	ld %r0, 0x50(%r1)
	addi %r1, %r1, 0x40
	mtlr %r0
	blr

4:

	li %r3, 0
	li %r11, 255
	sc 1

	b 4b
