  .data

cmsg:
  .asciz "Hello from ARM64!\n"
  .align 8
cmsg_end:

  .text

  .global hello_world
  .type hello_world, %function
hello_world:
  mov x0, #1 
  adr x1, cmsg
  adrp x2, cmsg_end
  add x2, x2, :lo12:cmsg_end
  sub x2, x2, x1
  mov x8, #64
  svc #0
  ret

  .global util_strlen
  .type util_strlen, %function
util_strlen:
  mov x1, #0          // counter = 0 (x1)
.loop:
    ldrb w2, [x0]     // w2 = *s (byte)
    cbz w2, .ret      // if zero -> return
    add x1, x1, #1
    add x0, x0, #1
    b .loop
.ret:
  mov x0, x1          // return in x0
  ret

	.global imath_rand_uint64
	.type imath_rand_uint64, @function
imath_rand_uint64:
	/* read ID_AA64ISAR0_EL1 into x1 */
	mrs	x1, ID_AA64ISAR0_EL1
	/* RNDR bitfield is bits [63:60] per earlier code; RNDR != 0 means RNDR support */
	lsr	x2, x1, #60
	and	x2, x2, #0xf
	cbz	x2, .use_cntvct
	/* RNDR read system register s3_3_c2_c4_0 into x0 */
	mrs	x0, s3_3_c2_c4_0
	b	.scramble
.use_cntvct:
	mrs	x0, cntvct_el0
.scramble:
	movz	x1, #0x4421, lsl #0	 /* placeholder; easier: perform multiply via movabs */
	/* Simpler: load constant from literal pool */
	adrp	x1, .LC
	add	x1, x1, :lo12:.LC
	ldr	x1, [x1]
	mul	x0, x0, x1
	ret
	.data
.LC:
	.quad 0x9a9fbe3f8f21421f
