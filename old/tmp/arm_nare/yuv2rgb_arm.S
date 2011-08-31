/*
 * Copyright (c) 2008, Movenda S.p.A. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer. Redistributions in binary form must
 * reproduce the above copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution. Neither the name of the Movenda S.p.A. nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission. THIS SOFTWARE IS PROVIDED BY
 * THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
.text

.macro load_uv
	// Load BU
	ldrb r0, [r9]
	add r9, r9, #1
	add r5, r3, #2048
	add r0, r5, r0, lsl #3
	ldmia r0, { r8, r14 }

	// Load BV
	ldrb r0, [r10]
	add r10, r10, #1
	add r6, r5, #2048
	add r0, r6, r0, lsl #3
	ldmia r0, { r5, r6 }

	// Create BUV, store in r8 and r14 - we need these value forever
	//sadd16 r8, r8, r5
	add r8,r5
	cmp r8, #0xffff
	bgt r8greater
	b r8notgreat
	
r8greater:
	mov r8, #0xffff
r8notgreat:
	
	//sadd16 r14, r14, r6
	add r14,r6
	cmp r14, #0xffff
	bgt r14greater
	b r14notgreat
	
r14greater:
	mov r14, #0xffff
r14notgreat:
	
.endm

.macro process_pixel
	ldrb r0, [$0, $2]
	pld [$0, $2+1] 
	add r0, r3, r0, lsl #3 
	ldmia r0, { r5, r6 } 
	//sadd16 r5, r8, r5 
	add r5,r8
	cmp r5, #0xffff
	bgt r5greater
	b r5notgreat
	
r5greater:
	mov r5, #0xffff
r5notgreat:
	
	//sadd16 r6, r14, r6
	add r6,r14
	cmp r6, #0xffff
	bgt r6greater
	b r6notgreat
	
r6greater:
	mov r6, #0xffff
r6notgreat:
	 
	sxth r0, r5, ror #16
	sxth r5, r5
	mov r5, r5, asr #6
	pkhbt r5, r5, r0, lsl #10
	
	sxth r0, r6, ror #16
	sxth r6, r6
	mov r6, r6, asr #6
	pkhbt r6, r6, r0, lsl #10
	
	usat16 r5, #8, r5 
	usat16 r6, #8, r6 
	orr r5, r5,r6, lsl #8 
	stmia $1!, {r5}
.endm

.globl _mb_YUV420_to_RGB32

// __mb_YUV420_to_RGB32(r0 = size, r1 = lumaStep, r2 = *yuv[], r3 = *dest)
_mb_YUV420_to_RGB32:
	stmdb sp!, { r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
	
	// r4 = width, r5 = height
	//uxth r4, r0, ror #16
	//uxth r5, r0
	push {r6, lr}
	mov r6,#0x0000ffff
	mov r4,r0
	ror r4,#16
	and r4,r6
		
	mov r5,r0
	and r5,r6
	pop {r6, pc}
	
//	r8 = offset between y lines;	
	sub r8, r1, r4
	
//	r3 = dest + (width * height * 4) = dst
	mul r0, r4, r5
	add r3, r3, r0, lsl #2

//  loading r2 = y, r9 = u, r10 = v
	ldmia r2, { r2, r9, r10 }
	add r1, r1, r2
	
// r6 = row counter
	mov r6, #0
vloop:
	// load d, d1
	sub r11, r3, r4, lsl #2
	sub r12, r11, r4, lsl #2
	mov r3, r12

// r7 = col counter
	mov r7, #0
	stmdb sp!, { r3, r5, r6, r8 }
	adr r3, RGBYUV_LOC
	ldr r3, [r3]
	
hloop:
	load_uv
	process_pixel r2,r11,#0
	process_pixel r2, r11, #1
	add r2, r2, #2
		
	process_pixel r1, r12, #0
	process_pixel r1, r12, #1
	add r1, r1, #2
		
	cmp r7, r4, lsr #1
	add r7, r7, #1
	cmp r7, r4, lsr #1
	blt hloop
		
	ldmia sp!, { r3, r5, r6, r8 }
	add r9, r9, r8, lsr #1
	add r10, r10, r8, lsr #1
	
	// change y,y1
	sub r0, r1, r2
	add r0, r0, r8
	add r2, r2, r0
	add r1, r1, r0
	
	add r6, r6, #1
	cmp r6, r5, lsr #1
	blt vloop
	
exit:
	ldmia sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}

RGBYUV_LOC: .long _RGBYUV
