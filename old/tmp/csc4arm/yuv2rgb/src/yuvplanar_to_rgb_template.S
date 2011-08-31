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

#define in_y			r0
#define in_u			r1
#define in_v			r2
#define outptr			r3
#define tmp1			r4

#define Y				r5
#define U 				r6
#define V				r7
#define Rm				r8
#define Gm				r9
#define Bm				r10
#define c1				r11
#define c2				r12

#define k1				c1
#define k2				c1
#define k3				c2
#define k4				c2

#define tmp				r14

	.macro	addsat8,dst,x,y
	adds	\dst, \x, \y, asr #PRECISION_SHIFT
	movmi	\dst, #0
	cmp		\dst, #255
	movgt	\dst, #255
	.endm

	.macro reload_ms shift
	mov		Rm, #0xff
	and		tmp, Rm, U, lsr #\shift
	sub		tmp, #128
	and		tmp1, Rm, V, lsr #\shift
	sub		tmp1, #128

	smulbb	Rm, tmp1, k1	@ 1.402 * (Cr - 128)

	smulbt	Gm, tmp, k2		@ -0.34414 * (Cb - 128)
	smlabb	Gm, tmp1, k3, Gm@ -0.71414 * (Cr - 128)

	smulbt	Bm, tmp, k4		@ 1.772 * (Cb - 128)
	.endm

	.macro	put_rgb	_Y, _Rm, _Gm, _Bm, _outptr, _offset
	addsat8	tmp, \_Y, \_Rm
	strb	tmp, [\_outptr, #\_offset + R_OFFSET]
	addsat8	tmp, \_Y, \_Gm
	strb	tmp, [\_outptr, #\_offset + G_OFFSET]
	addsat8	tmp, \_Y, \_Bm
	strb	tmp, [\_outptr, #\_offset + B_OFFSET]
	.endm



	.text
@ (const u8 *src_y, const u8 *src_u, const u8 *src_v, u8 *dst, u32 width)
	.align	4
	.global	FUNC_NAME
FUNC_NAME:
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, lr}

	@ load immediate
.L0:
	add		tmp, pc, #.LWORD - .L0 - 8
	ldmfd	tmp, {c1, c2}

.L1:
	ldr		tmp, [sp, #36]
	subs	tmp, #PIXEL_GRANURALITY * 4
	ldmmifd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp, pc} @ width - 2 < 0
	@ N Flag
	str		tmp, [sp, #36]

.L2:
@ load pixel
	ldr		U, [in_u], #4
	ldr		V, [in_u], #4

	ldrb	Y, [in_y], #1
	reload_ms 0
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 0

	ldrb	Y, [in_y], #1
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 1 


	ldrb	Y, [in_y], #1
	reload_ms 8
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 2

	ldrb	Y, [in_y], #1
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 3


	ldrb	Y, [in_y], #1
	reload_ms 16
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 4

	ldrb	Y, [in_y], #1
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 5


	ldrb	Y, [in_y], #1
	reload_ms 24
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 6

	ldrb	Y, [in_y], #1
	put_rgb	Y, Rm, Gm, Bm, outptr, RGB_BPP * 7

.L3:
	add		outptr, #RGB_BPP * PIXEL_GRANURALITY * 4
	b		.L1
	.align	2
.LWORD:
	.short	C_1_402
	.short	-C_0_34414
	.short	-C_0_71414
	.short	C_1_772
.size FUNC_NAME,.-FUNC_NAME
