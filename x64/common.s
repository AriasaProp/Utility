  .data

cmsg:
  .asciz "Hello from x64!\n"
  .align 8
cmsg_end:

  .text

  .global hello_world
  .type hello_world, @function
hello_world:
  mov rax, 1          ; sys_write
  mov rdi, 1          ; fd = 1
  lea rsi, [rel cmsg] ; buf (RIP-relative) compute length: cmsg_end - cmsg
  lea rdx, [rel cmsg_end]
  sub rdx, rsi        ; rdx = length
  syscall
  ret

  .global util_strlen
  .type util_strlen, @function
util_strlen:                ; x64 System V
  xor rax, rax          ; counter = 0 (use RAX as counter)
.loop:
    mov bl, byte [rdi]  ; bl = *s
    test bl, bl
    je .ret
    inc rax
    inc rdi
    jmp .loop
.ret:
  ret

	.global imath_rand_uint64
	.type imath_rand_uint64, @function
imath_rand_uint64:
	xorq	%rax, %rax
	movl	$1, %eax
	cpuid
	/* ecx bit 30 indicates RDRAND availability */
	bt	$30, %ecx
	jnc	.rdrand_fallback
	/* Try RDRAND (RDRAND r64 sets CF on success) */
.rdrand_try:
	rdrandq %rax
	setc	%al
	testb	%al, %al
	jz	.rdrand_fallback
	/* rdrand placed 64-bit result in RAX already */
	jmp	.scramble
.rdrand_fallback:
	/* Use RDTSCP if available (CPUID leaf 0x80000001 not needed); RDTSCP writes TSC into EDX:EAX */
	rdtscp
	shlq	$32, %rdx
	orq	%rdx, %rax
.scramble:
	/* scramble: x *= 0x9a9fbe3f8f21421f */
	movq	%rax, %rdx
	movabs $0x9a9fbe3f8f21421f, %rcx
	imulq	%rcx, %rdx
	movq	%rdx, %rax
	ret

