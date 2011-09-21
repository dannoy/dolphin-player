/* Copyright (c) 2008, Nozomi SATO. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Nozomi Sato nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define PRECISION_SHIFT     14

#include "yuv_to_rgb_constants.h"

#define in_ptr			r0
#define outptr			r1
#define width			r2

#define Y1				r3
#define Y2				r4
#define U 				r5
#define V				r6
#define Rm				r7
#define Gm				r8
#define Bm				r9
#define c1				r10
#define c2				r11

#define k1				c1
#define k2				c1
#define k3				c2
#define k4				c2

#define tmp				r12

#define YUV_BPP 2


	.macro	get_yuv
#if (YUV_BPP == 2)
	ldr		U, [in_ptr]
	mov     tmp, #0xff
	and     Y1, tmp, U, lsl #Y1_SHIFT
	and     V,  tmp, U, lsl #V_SHIFT
	and     Y2, tmp, U, lsl #Y2_SHIFT
	and     U,  tmp, U, lsl #U_SHIFT
#else
#error "invalid YUV_BPP"
#endif
	.endm

	.text
@ (const u8 *src, u8 *dst, u32 width)
	.align	4
	.global	FUNC_NAME
FUNC_NAME:
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, lr}

	@ load immediate
.L0:
	add		tmp, pc, #.LWORD - .L0 - 8
	ldmfd	tmp, {c1, c2}

.L1:
	subs	width, #PIXEL_GRANURALITY
	ldmmifd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, pc} @ width - 2 < 0
	@ N Flag

.L2:
@ load pixel
	get_yuv

	sub		U, #128
	sub		V, #128

	smulbb	Rm, V, k1		@ 1.402 * (Cr - 128)

	smulbt	Gm, U, k2		@ -0.34414 * (Cb - 128)
	smlabb	Gm, V, k3, Gm	@ -0.71414 * (Cr - 128)

	smulbt	Bm, U, k4		@ 1.772 * (Cb - 128)

	.macro	addsat8,dst,x,y
	adds	\dst, \x, \y
	movmi	\dst, #0
	cmp		\dst, #255
	movgt	\dst, #255
	.endm

@ R1
	addsat8	tmp, Y1, Rm
	strb	tmp, [outptr, #R_OFFSET]

@ G1
	addsat8	tmp, Y1, Gm
	strb	tmp, [outptr, #G_OFFSET]

@ B1
	addsat8	tmp, Y1, Bm
	strb	tmp, [outptr, #B_OFFSET]

@ R2
	addsat8 tmp, Y2, Rm
	strb	tmp, [outptr, #R_OFFSET + RGB_BPP]

@ G2
	addsat8 tmp, Y2, Gm
	strb	tmp, [outptr, #G_OFFSET + RGB_BPP] 

@ B2
	addsat8 tmp, Y2, Bm
	strb	tmp, [outptr, #B_OFFSET + RGB_BPP]

.L3:
	add		in_ptr, #YUV_BPP * PIXEL_GRANURALITY
	add		outptr, #RGB_BPP * PIXEL_GRANURALITY
	b		.L1
	.align	2
.LWORD:
	.short	C_1_402
	.short	-C_0_34414
	.short	-C_0_71414
	.short	C_1_772
.size FUNC_NAME,.-FUNC_NAME
