#define MATH_SHA1_SMALL_STACK

#define EXPAND_FUNC
#define USE_LDMIA

; Note
;   The MATHSHA1Context structure's members currently have the following alignment.
;     h: 0 - 20
;     block: 20 - 84
;   Revisions are needed when the members change

;rfp	.req	r9
;sl	.req	r10
;fp	.req	r11
;ip	.req	r12
;sp	.req	r13
;lr	.req	r14
;pc	.req	r15
    .text
.L100:
.L101:
    .word 0x5A827999
.L102:
    .word 0x6ED9EBA1
.L103:
    .word 0x8F1BBCDC
.L104:
    .word 0xCA62C1D6
.text
	.align 0
	.globl MATHi_SHA1ProcessBlock
	.type MATHi_SHA1ProcessBlock,@function
MATHi_SHA1ProcessBlock:
	stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	; r3, lr  : General-purpose
    ; r4-r8   : a-e
	; r9      : t
	; r10     : tmp
	; r11     : r0 backup
	; r12     : Address of w

	; Back up the arguments
	mov r11, r0
	; Save the local variables
#if !defined(MATH_SHA1_SMALL_STACK)
	;      u32   w[80];
	sub sp, sp, #320
	mov r12, sp
#else
	;      u32   w[16];
	sub sp, sp, #64
	mov r12, sp
#endif

	; r3      : Constant for NETSwapBytes32
	; r9      : t
	; r11     : r0 backup
	; r12     : Address of w
	; r2, lr  : General-purpose
	; r4-r7   : Load values
	; r1, r8  : Address of w[t] and context->block[t]
	; r0,r1,r2,r8,r10 : not use
	mvn r3, #0x0000FF00
	add r8, r11, #20
	mov r1, r12
	ldmia r8!, {r4, r5, r6, r7}
	; Set t
    mov r9, #16
.L105:
	; w[t] = NETConvert32HToBE(((u32*)context->block)[t]);
	; w[t+1] = NETConvert32HToBE(((u32*)context->block)[t+1]);
	; w[t+2] = NETConvert32HToBE(((u32*)context->block)[t+2]);
	; w[t+3] = NETConvert32HToBE(((u32*)context->block)[t+3]);
	; Expansion of NETConvert32HToBE
	eor r2, r4, r4, ror #16
    and r2, r3, r2, lsr #8
    eor r4, r2, r4, ror #8

	; Expansion of NETConvert32HToBE
	eor r2, r5, r5, ror #16
    and r2, r3, r2, lsr #8
    eor r5, r2, r5, ror #8

	; Expansion of NETConvert32HToBE
	eor r2, r6, r6, ror #16
    and r2, r3, r2, lsr #8
    eor r6, r2, r6, ror #8

	; Expansion of NETConvert32HToBE
	eor r2, r7, r7, ror #16
    and r2, r3, r2, lsr #8
    eor r7, r2, r7, ror #8

	stmia r1!, {r4, r5, r6, r7}
	; for (t = 0; t < 16; t+=4)
	subs r9, r9, #4
	ldmneia r8!, {r4, r5, r6, r7}
	bne .L105

#if !defined(MATH_SHA1_SMALL_STACK)
	; r0 - r3, lr  : General-purpose
	; r8      : prev
	; r9      : t
	; r11     : r0 backup
	; r12     : Address of w
	; r4 - r7 : Value to write back to w[t]
	; r10     : not use
	mov r9, #16

	sub r8, r9, #16
	add r8, r12, r8, lsl #2
	ldr r1, [r8]
	ldr lr, [r8, #8]
.L106:
	; u32    *prev = &w[t - 16];
	; w[t] = NETRotateLeft32(1, prev[ 0] ^ prev[ 2] ^ prev[ 8] ^ prev[13]);
	ldr r3, [r8, #32]
	eor r1, r1, lr
	ldr r2, [r8, #52]
	eor r1, r1, r3
	eor r1, r1, r2
	; NETRotateLeft32
	mov r4, r1, ror #31

	; w[t+1] = NETRotateLeft32(1, prev[ 1] ^ prev[ 3] ^ prev[ 9] ^ prev[14]);
	ldr r1, [r8, #4]
	ldr lr, [r8, #12]
	ldr r3, [r8, #36]
	eor r1, r1, lr
	ldr r2, [r8, #56]
	eor r1, r1, r3
	eor r1, r1, r2
	; NETRotateLeft32
	mov r5, r1, ror #31

	; w[t+2] = NETRotateLeft32(1, prev[ 2] ^ prev[ 4] ^ prev[10] ^ prev[15]);
	ldr r1, [r8, #8]
	ldr lr, [r8, #16]
	ldr r3, [r8, #40]
	eor r1, r1, lr
	ldr r2, [r8, #60]
	eor r1, r1, r3
	eor r1, r1, r2
	; NETRotateLeft32
	mov r6, r1, ror #31

	; w[t+3] = NETRotateLeft32(1, prev[ 3] ^ prev[ 5] ^ prev[11] ^ prev[16]);
	ldr r1, [r8, #12]
	ldr lr, [r8, #20]
	ldr r3, [r8, #44]
	eor r1, r1, lr
	eor r1, r1, r3
	eor r1, r1, r4
	; NETRotateLeft32
	mov r7, r1, ror #31

	add lr, r12, r9, lsl #2
	stmia lr, {r4, r5, r6, r7}

	; for (; t < 80; ++t)
	add r9, r9, #4
    cmp r9, #80
	subne r8, r9, #16
	addne r8, r12, r8, lsl #2
	ldrne r1, [r8]
	ldrne lr, [r8, #8]
	bne .L106
#endif

    ; Load a,b,c,d,e
#if !defined(USE_LDMIA)
	ldr r4, [r11]
	ldr r5, [r11, #4]
	ldr r6, [r11, #8]
	ldr r7, [r11, #12]
	ldr r8, [r11, #16]
#else
	ldmia r11, {r4, r5, r6, r7, r8}
#endif

	; r3, r2, r1, lr  : General-purpose
    ; r4-r8   : a-e
	; r9      : t
	; r10     : tmp
	; r11     : r0 backup
	; sp      : Address of w
	; r12     : Fixed value
	ldr r12, .L101
	mov r9, #0
.L107:
    ; u32     tmp = 0x5A827999UL + ((b & c) | (~b & d));
	and r10, r5, r6
	mvn lr, r5
	and lr, lr, r7
	orr r10, r10, lr
	add r10, r10, r12
    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr lr, [sp, r9, lsl #2]
#else
	and lr, r9, #15
	ldr lr, [sp, lr, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, lr
	add r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

	; for (; t < 16; ++t)
	add r9, r9, #1
    cmp r9, #16
	bne .L107

.L108:
    ; u32     tmp = 0x5A827999UL + ((b & c) | (~b & d));
	and r10, r5, r6
	mvn lr, r5
	and lr, lr, r7
	orr r10, r10, lr
	add r10, r10, r12
#if defined(MATH_SHA1_SMALL_STACK)
    ; w_alias(t) = NETRotateLeft32(1, 
    ;                       w_alias(t - 16 +  0) ^
    ;                       w_alias(t - 16 +  2) ^
    ;                       w_alias(t - 16 +  8) ^
    ;                       w_alias(t - 16 + 13) );
	sub r2, r9, #16
	and r2, r2, #0xF
	sub lr, r9, #14
	and lr, lr, #0xF
	ldr r3, [sp, r2, lsl #2]
	ldr r1, [sp, lr, lsl #2]
	sub lr, r9, #8
	eor r3, r3, r1
	and lr, lr, #0xF
	ldr r1, [sp, lr, lsl #2]
	sub r2, r9, #3
	eor r3, r3, r1
	and r2, r2, #0xF
	ldr r1, [sp, r2, lsl #2]
	and r2, r9, #0xF
	eor r3, r3, r1
	mov r3, r3, ror #31
	str r3, [sp, r2, lsl #2]
#endif
    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr lr, [sp, r9, lsl #2]
#else
	and lr, r9, #15
	ldr lr, [sp, lr, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, lr
	add r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

	; for (; t < 20; ++t)
	add r9, r9, #1
    cmp r9, #20
	bne .L108

	ldr r12, .L102
.L109:
    ; u32     tmp = 0x6ED9EBA1UL + (b ^ c ^ d);
	eor r10, r5, r6
	eor r10, r10, r7
	add r10, r10, r12
#if defined(MATH_SHA1_SMALL_STACK)
    ; w_alias(t) = NETRotateLeft32(1, 
    ;                       w_alias(t - 16 +  0) ^
    ;                       w_alias(t - 16 +  2) ^
    ;                       w_alias(t - 16 +  8) ^
    ;                       w_alias(t - 16 + 13) );
	sub r2, r9, #16
	and r2, r2, #0xF
	sub lr, r9, #14
	and lr, lr, #0xF
	ldr r3, [sp, r2, lsl #2]
	ldr r1, [sp, lr, lsl #2]
	sub lr, r9, #8
	eor r3, r3, r1
	and lr, lr, #0xF
	ldr r1, [sp, lr, lsl #2]
	sub r2, r9, #3
	eor r3, r3, r1
	and r2, r2, #0xF
	ldr r1, [sp, r2, lsl #2]
	and r2, r9, #0xF
	eor r3, r3, r1
	mov r3, r3, ror #31
	str r3, [sp, r2, lsl #2]
#endif
    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr lr, [sp, r9, lsl #2]
#else
	and lr, r9, #15
	ldr lr, [sp, lr, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, lr
	add r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

	; for (; t < 40; ++t)
	add r9, r9, #1
    cmp r9, #40
	bne .L109

	ldr r12, .L103
.L110:
    ; u32     tmp = 0x8F1BBCDCUL + ((b & c) | (b & d) | (c & d));
	and r10, r5, r6
	and lr, r5, r7
	orr r10, r10, lr
	and lr, r6, r7
	orr r10, r10, lr
	add r10, r10, r12
#if defined(MATH_SHA1_SMALL_STACK)
    ; w_alias(t) = NETRotateLeft32(1, 
    ;                       w_alias(t - 16 +  0) ^
    ;                       w_alias(t - 16 +  2) ^
    ;                       w_alias(t - 16 +  8) ^
    ;                       w_alias(t - 16 + 13) );
	sub r2, r9, #16
	and r2, r2, #0xF
	sub lr, r9, #14
	and lr, lr, #0xF
	ldr r3, [sp, r2, lsl #2]
	ldr r1, [sp, lr, lsl #2]
	sub lr, r9, #8
	eor r3, r3, r1
	and lr, lr, #0xF
	ldr r1, [sp, lr, lsl #2]
	sub r2, r9, #3
	eor r3, r3, r1
	and r2, r2, #0xF
	ldr r1, [sp, r2, lsl #2]
	and r2, r9, #0xF
	eor r3, r3, r1
	mov r3, r3, ror #31
	str r3, [sp, r2, lsl #2]
#endif
    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr lr, [sp, r9, lsl #2]
#else
	and lr, r9, #15
	ldr lr, [sp, lr, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, lr
	add r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

	; for (; t < 60; ++t)
	add r9, r9, #1
    cmp r9, #60
	bne .L110

	ldr r12, .L104
.L111:
	; u32     tmp = 0xCA62C1D6UL + (b ^ c ^ d);
	eor r10, r5, r6
	eor r10, r10, r7
	add r10, r10, r12
#if defined(MATH_SHA1_SMALL_STACK)
    ; w_alias(t) = NETRotateLeft32(1, 
    ;                       w_alias(t - 16 +  0) ^
    ;                       w_alias(t - 16 +  2) ^
    ;                       w_alias(t - 16 +  8) ^
    ;                       w_alias(t - 16 + 13) );
	sub r2, r9, #16
	and r2, r2, #0xF
	sub lr, r9, #14
	and lr, lr, #0xF
	ldr r3, [sp, r2, lsl #2]
	ldr r1, [sp, lr, lsl #2]
	sub lr, r9, #8
	eor r3, r3, r1
	and lr, lr, #0xF
	ldr r1, [sp, lr, lsl #2]
	sub r2, r9, #3
	eor r3, r3, r1
	and r2, r2, #0xF
	ldr r1, [sp, r2, lsl #2]
	and r2, r9, #0xF
	eor r3, r3, r1
	mov r3, r3, ror #31
	str r3, [sp, r2, lsl #2]
#endif
    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr lr, [sp, r9, lsl #2]
#else
	and lr, r9, #15
	ldr lr, [sp, lr, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, lr
	add r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

	; for(t=60; t<80; ++t)
	add r9, r9, #1
    cmp r9, #80
	bne .L111

	; r3, r2, r1, lr  : General-purpose
	; r9, r10, r12    : General-purpose
    ; r4-r8   : a-e
	; r11     : r0 backup

    ; Increment a,b,c,d, and e in context->h
#if !defined(USE_LDMIA)
	ldr r3, [r11]
	add r3, r3, r4
	str r3, [r11]
	ldr r3, [r11, #4]
	add r3, r3, r5
	str r3, [r11, #4]
	ldr r3, [r11, #8]
	add r3, r3, r6
	str r3, [r11, #8]
	ldr r3, [r11, #12]
	add r3, r3, r7
	str r3, [r11, #12]
	ldr r3, [r11, #16]
	add r3, r3, r8
	str r3, [r11, #16]
#else
	ldmia r11, {r1, r2, r3, r9, r10}
	add r1, r1, r4
	add r2, r2, r5
	add r3, r3, r6
	add r9, r9, r7
	add r10, r10, r8
	stmia r11, {r1, r2, r3, r9, r10}
#endif

	; Free local variables
#if !defined(MATH_SHA1_SMALL_STACK)
	;      u32   w[80];
	add sp, sp, #320
#else
	;      u32   w[16];
	add sp, sp, #64
#endif

	ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}
.L112:
	.size	MATHi_SHA1ProcessBlock,.L112-.L100


#if !defined(EXPAND_FUNC)
; WARNING! This function does not comply with the APCS!
;        It cannot be used from the C programming language!
.text
    .align 0
    .globl rotate_values
    .type rotate_values,@function
rotate_values:
	; r3, r2  : General-purpose
    ; r4-r8   : a-e
	; r9      : t
	; r10     : tmp
	; r11     : r0 backup
	; r12     : Address of w
	; Conforms to the caller MATHi_SHA1ProcessBlock (the only change is from lr to r2)
	; Registers are not backed up

    ; tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
#if !defined(MATH_SHA1_SMALL_STACK)
	ldr r3, [r12, r9, lsl #2]
#else
	and r3, r9, #15
	ldr r3, [r12, r3, lsl #2]
#endif
	add r10, r10, r8
	add r10, r10, r3
	and r10, r10, r4, ror #27
    ; e = d;
	mov r8, r7
    ; d = c;
	mov r7, r6
    ; c = NETRotateLeft32(30, b);
	mov r6, r5, ror #2
    ; b = a;
	mov r5, r4
    ; a = tmp;
	mov r4, r10

    mov pc, lr
.L113:
    .size   rotate_values,.L113-rotate_values


#if defined(MATH_SHA1_SMALL_STACK)
; WARNING! This function does not comply with the APCS!
;        It cannot be used from the C programming language!
.text
    .align 0
    .globl update_values
    .type update_values,@function
update_values:
	; r3, r2, r1  : General-purpose
    ; r4-r8   : a-e
	; r9      : t
	; r10     : tmp
	; r11     : r0 backup
	; r12     : Address of w
	; Conforms to the caller MATHi_SHA1ProcessBlock (the only change is from lr to r2)
	; Registers are not backed up

    ; w_alias(t) = NETRotateLeft32(1, 
    ;                       w_alias(t - 16 +  0) ^
    ;                       w_alias(t - 16 +  2) ^
    ;                       w_alias(t - 16 +  8) ^
    ;                       w_alias(t - 16 + 13) );
	sub r2, r9, #16
	and r2, r2, #0xF
	ldr r3, [r12, r2, lsl #2]
	sub r2, r9, #14
	and r2, r2, #0xF
	ldr r1, [r12, r2, lsl #2]
	eor r3, r3, r1
	sub r2, r9, #8
	and r2, r2, #0xF
	ldr r1, [r12, r2, lsl #2]
	eor r3, r3, r1
	sub r2, r9, #3
	and r2, r2, #0xF
	ldr r1, [r12, r2, lsl #2]
	eor r3, r3, r1
	mov r3, r3, ror #31
	and r2, r9, #0xF
	str r3, [r12, r2, lsl #2]

    mov pc, lr
.L114:
    .size   update_values,.L114-update_values
#endif
#endif  ; !defined(EXPAND_FUNC)


#if 0
.text
    .align 0
    .globl NETSwapBytes32
    .type NETSwapBytes32,@function
NETSwapBytes32:
    ; r0:n, r1:m, r2:t
    mvn r1, #0x0000FF00
    eor r2, r0, r0, ror #16
    and r2, r1, r2, lsr #8
    eor r0, r2, r0, ror #8
    mov pc, lr
.L115:
    .size   NETSwapBytes32,.L115-NETSwapBytes32


.text
    .align 0
	.globl NETRotateLeft32
	.type NETRotateLeft32,@function
NETRotateLeft32:
	rsb r2, r0, #32
	mov r0, r1, ror r2
	mov pc, lr
.L116:
    .size   NETRotateLeft32,.L116-NETRotateLeft32
#endif
