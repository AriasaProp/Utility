  .data

cmsg:
  .asciz "Hello from x64!\n"
  .align 8
cmsg_end:

  .text

  .global hello_world
  .type hello_world, @function
hello_world:
  mov $1, %rax           # sys_write
  mov $1, %rdi           # fd = 1
  lea cmsg, %rsi         # compute length: cmsg_end - cmsg
  lea cmsg_end, %rdx
  sub %rsi, %rdx         # rdx = length
  syscall
  ret

  .global util_strlen
  .type util_strlen, @function
util_strlen:            # x64 System V
  xor %rax, %rax        # counter = 0 (use RAX as counter)
.loop:
    mov %bl, 8(%rdi)  # bl = *s
    test %bl, %bl
    je .ret
    inc %rax
    inc %rdi
    jmp .loop
.ret:
  ret

	.global imath_rand_uint64
	.type imath_rand_uint64, @function
imath_rand_uint64:
	xor	%rax, %rax
	mov	$1, %eax
	cpuid
	bt	$30, %ecx
	jnc	.fallback
	rdrandq %rax
	setc	%al
	testb	%al, %al
	jz	.fallback
	jmp	.scramble
.fallback:
	rdtscp
	shlq	$32, %rdx
	orq	%rdx, %rax
.scramble:
	movq	%rax, %rdx
	movabs $0x9a9fbe3f8f21421f, %rcx
	imulq	%rcx, %rdx
	movq	%rdx, %rax
	ret

