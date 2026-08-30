; Note
;   The MATHSHA256Context structure's members currently have the following alignment.
;     h: 0 - 32
;     data: 40 - 104
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
	.extern MATH_SHA256iConst
	.word MATH_SHA256iConst
.text
	.align 0
	.globl MATH_SHA256iProcessBlock
	.type MATH_SHA256iProcessBlock,@function
MATH_SHA256iProcessBlock:
	stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}

	; Save the context argument
	stmfd sp!,{r0}
	; Save the local variables
	;      u32   w[64];
	sub sp, sp, #256

	; t = 0;
	mov r12, #0

	; r0      : Address of context
	; r2      : General-purpose
	; r3      : General-purpose
	; r4      : Constant for NETConvert32HToBE
	; r5-r8   : data[t],w[t]
    ; r5-r11  : General-purpose
	; r12     : t
	; lr      : General-purpose
	; sp      : Address of w

	mvn r4, #0x0000FF00
.L102:
	; w[t] = NETConvert32HToBE(((u32*)context->data)[t]);
	add lr, r0, #40
	add lr, lr, r12, lsl #2
	ldmia lr, {r5, r6, r7, r8}
	; NETConvert32HToBE
	eor r2, r5, r5, ror #16
	and r2, r4, r2, lsr #8
	eor r5, r2, r5, ror #8
	; NETConvert32HToBE
	eor r2, r6, r6, ror #16
	and r2, r4, r2, lsr #8
	eor r6, r2, r6, ror #8
	; NETConvert32HToBE
	eor r2, r7, r7, ror #16
	and r2, r4, r2, lsr #8
	eor r7, r2, r7, ror #8
	; NETConvert32HToBE
	eor r2, r8, r8, ror #16
	and r2, r4, r2, lsr #8
	eor r8, r2, r8, ror #8
	add lr, sp, r12, lsl #2
	stmia lr, {r5, r6, r7, r8}

	; for (t = 0; t < 16; ++t)
	add r12, r12, #4
    cmp r12, #16
	bne .L102

	; r0      : Address of context
	; r2      : w[t]
	; r3-r6   : Loaded prev value
	; r7      : not use
	; r8-r10  : Loaded prev value
	; r11     : General-purpose
	; r12     : t
	; lr      : General-purpose
	; sp      : Address of w

.L103:
    ; prev = &w[t - 16];
	add lr, sp, r12, lsl #2
	sub lr, lr, #64
    ; w[t] = prev[ 0];
	ldr r3, [lr]
	ldr r4, [lr, #4]
	ldr r8, [lr, #8]
	ldr r5, [lr, #36]
	ldr r9, [lr, #40]
	ldr r6, [lr, #56]
	ldr r10, [lr, #60]
	; w[t] += SHA256_SSIGMA_0(prev[ 1]);
	;       = SHA256_ROTR(7,prev[ 1]) ^ SHA256_ROTR(18,prev[ 1]) ^ SHA256_SHR(3,prev[ 1]);
    mov r11, r4, lsl #25
	orr lr, r11, r4, lsr #7
    mov r11, r4, lsl #14
	orr r11, r11, r4, lsr #18
	eor lr, lr, r11
	eor lr, lr, r4, lsr #3
	add r2, r3, lr
    ; w[t] += prev[ 9];
	add r2, r2, r5
    ; w[t] += SHA256_SSIGMA_1(prev[14]);
	;       = SHA256_ROTR(17,prev[14]) ^ SHA256_ROTR(19,prev[14]) ^ SHA256_SHR(10,prev[14]);
    mov r11, r6, lsl #15
	orr lr, r11, r6, lsr #17
    mov r11, r6, lsl #13
	orr r11, r11, r6, lsr #19
	eor lr, lr, r11
	eor lr, lr, r6, lsr #10
	add r2, r2, lr
	; Write back w[t]
	str r2, [sp, r12, lsl #2]

	add r12, r12, #1

    ; prev = &w[t - 16];
    ; w[t] = prev[ 0];
	; w[t] += SHA256_SSIGMA_0(prev[ 1]);
	;       = SHA256_ROTR(7,prev[ 1]) ^ SHA256_ROTR(18,prev[ 1]) ^ SHA256_SHR(3,prev[ 1]);
    mov r11, r8, lsl #25
	orr lr, r11, r8, lsr #7
    mov r11, r8, lsl #14
	orr r11, r11, r8, lsr #18
	eor lr, lr, r11
	eor lr, lr, r8, lsr #3
	add r2, r4, lr
    ; w[t] += prev[ 9];
	add r2, r2, r9
    ; w[t] += SHA256_SSIGMA_1(prev[14]);
	;       = SHA256_ROTR(17,prev[14]) ^ SHA256_ROTR(19,prev[14]) ^ SHA256_SHR(10,prev[14]);
    mov r11, r10, lsl #15
	orr lr, r11, r10, lsr #17
    mov r11, r10, lsl #13
	orr r11, r11, r10, lsr #19
	eor lr, lr, r11
	eor lr, lr, r10, lsr #10
	add r2, r2, lr
	; Write back w[t]
	str r2, [sp, r12, lsl #2]

	; for (; t < 64; ++t)
	add r12, r12, #1
    cmp r12, #64
	bne .L103

	; Load a-h
	ldmia r0, {r4, r5, r6, r7, r8, r9, r10, r11}

	; t = 0;
	mov r12, #0

	; r0      : General-purpose
	; r1      : Address of MATH_SHA256iConst
	; r2      : tmp1
	; r3      : tmp2
    ; r4-r11  : a-h
	; r12     : t
	; lr      : General-purpose
	; sp      : Address of w

	ldr r1, .L101
	ldr lr, [sp, r12, lsl #2]
	ldr r0, [r1, r12, lsl #2]
.L104:
	; tmp1 = MATH_SHA256iConst[t];
	; tmp1 += w[t];
	add r2, r0, lr
	; tmp1 += h;
	; tmp1 += SHA256_LSIGMA_1(e);
	;       = SHA256_ROTR(6,e) ^ SHA256_ROTR(11,e) ^ SHA256_ROTR(25,e))
	mov lr, r8, lsl #26
	orr lr, lr, r8, lsr #6
	mov r0, r8, lsl #21
	orr r0, r0, r8, lsr #11
	eor lr, lr, r0
	mov r0, r8, lsl #7
	orr r0, r0, r8, lsr #25
	eor lr, lr, r0
	add r2, r2, r11
	add r2, r2, lr
	; tmp1 += SHA256_CH(e,f,g);
	;       = (((e)&(f))^((~e)&(g)))
	and lr, r8, r9
	mvn r0, r8
	and r0, r0, r10
	eor lr, lr, r0
	add r2, r2, lr

	; tmp2 = SHA256_LSIGMA_0(a);
	;      = SHA256_ROTR(2,a) ^ SHA256_ROTR(13,a) ^ SHA256_ROTR(22,a)
	mov lr, r4, lsl #30
	orr lr, lr, r4, lsr #2
	mov r0, r4, lsl #19
	orr r0, r0, r4, lsr #13
	eor lr, lr, r0
	mov r0, r4, lsl #10
	orr r0, r0, r4, lsr #22
	eor r3, lr, r0
	; tmp2 += SHA256_MAJ(a,b,c);
	;       = ((a)&(b))^((a)&(c))^((b)&(c))
	and lr, r4, r5
	and r0, r4, r6
	eor lr, lr, r0
	and r0, r5, r6
	eor lr, lr, r0
	add r3, r3, lr

    ; h = g;
	mov r11, r10
    ; g = f;
	mov r10, r9
    ; f = e;
	mov r9, r8
    ; e = d + tmp1;
	add r8, r7, r2
    ; d = c;
	mov r7, r6
    ; c = b;
	mov r6, r5
    ; b = a;
	mov r5, r4
    ; a = tmp1 + tmp2;
	add r4, r2, r3

	; for (t = 0; t < 64; ++t)
	add r12, r12, #1
    cmp r12, #64
	ldrne r0, [r1, r12, lsl #2]
	ldrne lr, [sp, r12, lsl #2]
	bne .L104

	; Free local variables
	;      u32   w[64];
	add sp, sp, #256

	; r0      : context argument
    ; r4-r11  : a-h
	; r1-r3,r12,lr : General-purpose

	; Load the context argument
	ldmfd sp!,{r0}
	; Increment a-h in context->h
	ldmia r0, {r1, r2, r3, r12}
	add r1, r1, r4
	add r2, r2, r5
	add r3, r3, r6
	add r12, r12, r7
	stmia r0!, {r1, r2, r3, r12}
	ldmia r0, {r1, r2, r3, r12}
	add r1, r1, r8
	add r2, r2, r9
	add r3, r3, r10
	add r12, r12, r11
	stmia r0!, {r1, r2, r3, r12}

	ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}
.L105:
	.size	MATH_SHA256iProcessBlock,.L105-.L100



#if 0
.text
    .align 0
	.globl SHA256_ROTR
	.type SHA256_ROTR,@function
SHA256_ROTR:
    ; r0     : Argument and return value
	; r1     : Argument
	; r2     : General-purpose
	; r3     : General-purpose
	rsb r0, r0, #32
    mov r2, r1, lsl r0
	rsb r3, r0, #32
	mov r3, r1, lsr r3
	orr r0, r2, r3
	mov pc, lr
.L106:
    .size   SHA256_ROTR,.L106-SHA256_ROTR


.text
    .align 0
	.globl NETRotateLeft32
	.type NETRotateLeft32,@function
NETRotateLeft32:
    ; r0     : Argument and return value
	; r1     : Argument
	; r2     : General-purpose
	; r3     : General-purpose
    mov r2, r1, lsl r0
	rsb r3, r0, #32
	mov r3, r1, lsr r3
	orr r0, r2, r3
	mov pc, lr
.L107:
    .size   NETRotateLeft32,.L107-NETRotateLeft32


.text
    .align 0
	.globl NETConvert32HToBE
	.type NETConvert32HToBE,@function
NETConvert32HToBE:
    ; r0     : Argument and return value
	; r1     : General-purpose
	; r2     : General-purpose
#if 1
	mvn r1, #0x0000FF00
	eor r2, r0, r0, ror #16
	and r2, r1, r2, lsr #8
	eor r0, r2, r0, ror #8
#else
	eor r1, r0, r0, ror #16
	mov r1, r1, lsr #8
	bic r1, r1, #0xff00
	eor r0, r1, r0, ror #8
#endif
	mov pc, lr
.L108:
    .size   NETConvert32HToBE,.L108-NETConvert32HToBE
#endif
