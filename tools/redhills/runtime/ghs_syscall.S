/*
 * The Green Hills host-syscall entry the Cafe libsys port calls (source/cafe/libsys/ind_io.cpp), for the
 * RedHills build. Transcribed from the two libghssys.a members the retail image links, which are
 * byte-identical to its .syscall __dotsyscall and its __ghs_syscall: build tooling, like the crt0, not
 * reconstructed game source.
 *
 * __dotsyscall is the address an attached debugger intercepts; on its own it returns at once. __ghs_syscall
 * sets cr0[eq] beforehand, so an unintercepted call reports success and returns its first argument; a
 * debugger that fails the call clears cr0[eq] and leaves the error code in r3, which goes to coreinit's
 * __gh_set_errno before -1 is returned.
 */

	.section .syscall, "ax", @progbits
	.align	2
	.globl	__dotsyscall
	.type	__dotsyscall, @function
__dotsyscall:
	nop
	blr
	.size	__dotsyscall, . - __dotsyscall

	.text
	.align	2
	.globl	__ghs_syscall
	.type	__ghs_syscall, @function
__ghs_syscall:
	mflr	%r0
	stw	%r0, 4(%r1)
	stwu	%r1, -8(%r1)
	crset	2
	bl	__dotsyscall
	beq	.Lnoerr
	bl	__gh_set_errno
	li	%r3, -1
.Lnoerr:
	lwz	%r0, 12(%r1)
	mtlr	%r0
	addi	%r1, %r1, 8
	blr
	.size	__ghs_syscall, . - __ghs_syscall
