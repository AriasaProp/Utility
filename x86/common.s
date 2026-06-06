  .data
cmsg:
  .ascii "Hello from x86!\n"
cmsg_end:

  .text

  .global hello_world
  .type hello_world, @function
hello_world:
  push %ebp
  mov %esp, %ebp
  mov $4, %eax        ; sys_write
  mov $1, %ebx        ; fd = 1 (stdout)
  lea cmsg, %ecx,     ; buf
  mov cmsg_end, %edx
  sub %ecx, %edx        ; compute length: cmsg_end - cmsg
  int 0x80

  mov %ebp, %esp
  pop %ebp
  ret

  .global util_strlen
  .type util_strlen, @function
util_strlen:          ; 32bit cdecl
  push %ebp
  mov %esp, %ebp
  mov 8(%ebp), %eax   ; eax = s (pointer)
  xor %ecx, %ecx        ; counter = 0
.loop:
  mov 8(%eax), %dl  ; dl = *s
  cmp $0, %dl
  je .ret
  inc %ecx
  inc %eax
  jmp .loop
.ret:
  mov %ecx, %eax       ; return = counter (in EAX)
  pop %ebp
  ret

	.global imath_rand_uint64
	.type imath_rand_uint64, @function
imath_rand_uint64:
	rdtsc
	pushl	%edx
	pushl	%eax
	/* Load low and high words */
	movl	8(%esp), %eax    /* low (original EAX) */
	movl	12(%esp), %edx   /* high (original EDX) */
	/* constant split: C_lo = 0x8f21421f, C_hi = 0x9a9fbe3f */
	/* compute low = eax * C_lo (32x32 -> 64) */
	movl	$0x8f21421f, %ecx
	mull	%ecx            /* EDX:EAX = eax * C_lo */
	/* save low product on stack */
	movl	%eax, -4(%esp)
	movl	%edx, -8(%esp)

	/* compute cross1 = edx * C_lo */
	movl	%edx, %eax
	mull	%ecx            /* EDX:EAX = edx * C_lo */
	/* add to middle */
	addl	-4(%esp), %edx  /* high part of first product + low of cross1 */
	adc	$0, %eax

	/* compute cross2 = eax_original * C_hi */
	/* reload original low into eax */
	movl	8(%esp), %eax
	movl	$0x9a9fbe3f, %ecx
	mull	%ecx            /* EDX:EAX = eax_orig * C_hi */
	addl	%eax, %edx
	adc	$0, %ecx         /* ecx used as temp, carry into ecx */
	/* top part = edx (from previous cross) + EDX (from this mul) + carry */
	movl	12(%esp), %eax   /* original high */
	mull	%ecx             /* eax * C_hi -> EDX:EAX */
	/* final high = previous high sums + EAX from this mul + EDX from earlier adds */
	/* For simplicity, we won't attempt perfect 64-bit full multiply carry chain here.
	   Instead produce a decent mixed value by combining partial results. */

	/* Build result low/high from earlier stored values */
	movl	-4(%esp), %eax
	movl	-8(%esp), %edx

	addl	$16, %esp
	ret
