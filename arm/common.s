  .data

cmsg:
  .ascii "Hello from ARM!\n"
cmsg_end:

	.align 2
seed:
	.word 0x12345678
	.word 0x9abcdef0

  .text

  .global hello_world
  .type hello_world, %function
hello_world:
  push {lr}           @ save return address
  mov r0, #1          @ fd = 1 (stdout)
  ldr r1, =cmsg       @ buf = address of cmsg
  ldr r2, =cmsg_end   @ buf = address of cmsg_end
  sub r2, r2, r1      @ count = cmsg_end - cmsg
  mov r7, #4          @ sys_write syscall number
  svc #0
  pop {pc}            @ return


  .global util_strlen
  .type util_strlen, %function
util_strlen:
  mov r1, #0          @ counter = 0 (r1)
.loop:
    ldrb r2, [r0]     @ r2 = *s
    cmp r2, #0
    beq .ret
    add r1, r1, #1
    add r0, r0, #1
    b .loop
.ret:
  mov r0, r1          @ return in r0
  bx lr
  	.text
	.align  2
	.global imath_rand_uint64
	.type   imath_rand_uint64, %function

imath_rand_uint64:
	ldr	r2, .Lseed_ptr        /* load address of seed */
	ldr	r3, [r2]              /* seed_lo -> r3 */
	ldr	r4, [r2, #4]          /* seed_hi -> r4 */
	/* update low 32: x = 1664525*x + 1013904223 */
	mov	r5, #1664525
	umull	r6, r7, r3, r5       /* r7:r6 = r3 * 1664525 */
	add	r6, r6, #1013904223
	mov	r8, r6               /* first output low */
	/* update high 32 with different LCG constants */
	mov	r5, #22695477
	umull	r9, r10, r4, r5
	add	r9, r9, #1103515245
	mov	r11, r9              /* updated high */
	/* store updated seed (volatile) */
	str	r8, [r2]
	str	r11, [r2, #4]
	/* second step on low for more mixing */
	mov	r3, r8
	mov	r5, #1664525
	umull	r6, r7, r3, r5
	add	r6, r6, #1013904223
	/* return low=r8, high=r6 */
	mov	r0, r8
	mov	r1, r6
	bx	lr
	/* literal for ldr of seed address */
	.align 2
.Lseed_ptr:
	.word seed
	.word seed + 4
	.size imath_rand_uint64, .-imath_rand_uint64



