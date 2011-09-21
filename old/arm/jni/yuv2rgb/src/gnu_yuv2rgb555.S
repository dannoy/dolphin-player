@ YUV-> RGB conversion code Copyright (C) 2008 Robin Watts (robin;wss.co.uk).
@
@ Licensed under the GPL. If you need it under another license, contact me
@ and ask.
@
@  This program is free software ; you can redistribute it and/or modify
@  it under the terms of the GNU General Public License as published by
@  the Free Software Foundation ; either version 2 of the License, or
@  (at your option) any later version.
@
@  This program is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY ; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with this program ; if not, write to the Free Software
@  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
@
@
@ The algorithm used here is based heavily on one created by Sophie Wilson
@ of Acorn/e-14/Broadcomm. Many thanks.
@
@ Additional tweaks (in the fast fixup code) are from Paul Gardiner.
@
@ The old implementation of YUV -> RGB did:
@
@ R = CLAMP((Y-16)*1.164 +           1.596*V)
@ G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
@ B = CLAMP((Y-16)*1.164 + 2.018*U          )
@
@ Were going to bend that here as follows:
@
@ R = CLAMP(y +           1.596*V)
@ G = CLAMP(y - 0.383*U - 0.813*V)
@ B = CLAMP(y + 1.976*U          )
@
@ where y = 0               for       Y <=  16,
@       y = (  Y-16)*1.164, for  16 < Y <= 239,
@       y = (239-16)*1.164, for 239 < Y
@
@ i.e. We clamp Y to the 16 to 239 range (which it is supposed to be in
@ anyway). We then pick the B_U factor so that B never exceeds 511. We then
@ shrink the G_U factor in line with that to avoid a colour shift as much as
@ possible.
@
@ Were going to use tables to do it faster, but rather than doing it using
@ 5 tables as as the above suggests, were going to do it using just 3.
@
@ We do this by working in parallel within a 32 bit word, and using one
@ table each for Y U and V.
@
@ Source Y values are    0 to 255, so    0.. 260 after scaling
@ Source U values are -128 to 127, so  -49.. 49(G), -253..251(B) after
@ Source V values are -128 to 127, so -204..203(R), -104..103(G) after
@
@ So total summed values:
@ -223 <= R <= 481, -173 <= G <= 431, -253 <= B < 511
@
@ We need to pack R G and B into a 32 bit word, and because of Bs range we
@ need 2 bits above the valid range of B to detect overflow, and another one
@ to detect the sense of the overflow. We therefore adopt the following
@ representation:
@
@ osGGGGGgggggosBBBBBbbbosRRRRRrrr
@
@ Each such word breaks down into 3 ranges.
@
@ osGGGGGggggg   osBBBBBbbb   osRRRRRrrr
@
@ Thus we have 8 bits for each B and R table entry, and 10 bits for G (good
@ as G is the most noticable one). The s bit for each represents the sign,
@ and o represents the overflow.
@
@ For R and B we pack the table by taking the 11 bit representation of their
@ values, and toggling bit 10 in the U and V tables.
@
@ For the green case we calculate 4*G (thus effectively using 10 bits for the
@ valid range) truncate to 12 bits. We toggle bit 11 in the Y table.

@ Theorarm library
@ Copyright (C) 2009 Robin Watts for Pinknoise Productions Ltd

    .text

	.global	yuv420_2_rgb555

@ void yuv420_to_rgb565
@  uint8_t *x_ptr
@  uint8_t *y_ptr
@  uint8_t *u_ptr
@  uint8_t *v_ptr
@  int      height
@  int      width
@  int      y_span
@  int      uv_span

 .set DITH1,	6
 .set DITH2,	7

yuv420_2_rgb555:

	@ r0 = x_ptr
	@ r1 = y_ptr
	@ r2 = u_ptr
	@ r3 = v_ptr
	@ <> = height
	@ <> = width
	@ <> = y_span
	@ <> = uv_span
	STMFD	r13!,{r4-r11,r14}

	LDR	r8, [r13,#9*4]		@ r8 = height
	LDR	r9, [r13,#10*4]		@ r9 = width
	LDR	r10,[r13,#11*4]		@ r10= y_span
	LDR	r4, =0x07C07C1F
	LDR	r5, =0x40040100
	MOV	r9, r9, LSL #1
yloop1:
	SUB	r8, r8, r9, LSL #15	@ r8 = height-(width<<16)
xloop1:
	LDRB	r11,[r2], #1		@ r11 = u  = *u_ptr++
	LDRB	r12,[r3], #1		@ r12 = v  = *v_ptr++
	LDRB	r7, [r1, r10]		@ r7  = y2 = y_ptr[stride]
	LDRB	r6, [r1], #1		@ r6  = y0 = *y_ptr++
	ADR	r14,u_table
	LDR	r11,[r14,r11,LSL #2]	@ r11 = u  = u_table[u]
	ADD	r14,r14,#1024		@ r14 = v_table
	LDR	r12,[r14,r12,LSL #2]	@ r12 = v  = v_table[v]
	ADD	r14,r14,#1024		@ r14 = y_table
	LDR	r7, [r14,r7, LSL #2]	@ r7  = y2 = y_table[y2]
	LDR	r6, [r14,r6, LSL #2]	@ r6  = y0 = y_table[y0]
	ADD	r11,r11,r12		@ r11 = uv = u+v

	ADD	r12,r11,r5, LSR #DITH1	@ (dither 1/4)
	ADD	r7, r7, r12		@ r7  = y0 + uv
	ADD	r6, r6, r12		@ r6  = y2 + uv
	ADD	r6, r6, r5, LSR #DITH2	@ (dither 3/4)
	ANDS	r12,r7, r5
	TSTEQ	r6, r5
	BNE	fix101
return101:
	AND	r7, r4, r7, LSR #3
	ORR	r7, r7, r7, LSR #17
	BIC	r7,r7,#0x8000
	STRH	r7, [r0, r9]
	AND	r6, r4, r6, LSR #3
	ORR	r6, r6, r6, LSR #17
	LDRB	r12,[r1, r10]		@ r12 = y3 = y_ptr[stride]
	LDRB	r7, [r1], #1		@ r6  = y1 = *y_ptr++
	BIC	r6,r6,#0x8000
	STRH	r6, [r0], #2

	LDR	r12,[r14, r12,LSL #2]	@ r7  = y3 = y_table[y2]
	LDR	r6, [r14, r7, LSL #2]	@ r6  = y1 = y_table[y0]

	ADD	r7, r12,r11		@ r7  = y3 + uv
	ADD	r6, r6, r11		@ r6  = y1 + uv
	ADD	r7, r7, r5, LSR #DITH2	@ (dither 2/4)
	ANDS	r12,r7, r5
	TSTEQ	r6, r5
	BNE	fix102
return102:
	AND	r7, r4, r7, LSR #3
	AND	r6, r4, r6, LSR #3
	ORR	r7, r7, r7, LSR #17
	ORR	r6, r6, r6, LSR #17
	BIC	r7,r7,#0x8000
	BIC	r6,r6,#0x8000
	STRH	r7, [r0, r9]
	STRH	r6, [r0], #2
	ADDS	r8, r8, #2<<16
	BLT	xloop1

	LDR	r11,[r13,#12*4]		@ r11 = uv_stride
	@ADD	r0, r0, #2*X_STRIDE - 2*WIDTH	; x_ptr to next line
	@ADD	r1, r1, #2*Y_STRIDE -   WIDTH	; y_ptr to next line
	ADD	r0, r0, r9
	ADD	r1, r1, r10,LSL #1
	SUB	r1, r1, r9, LSR #1
	SUB	r2, r2, r9, LSR #2
	SUB	r3, r3, r9, LSR #2
	ADD	r2, r2, r11
	ADD	r3, r3, r11

	SUBS	r8, r8, #2
	BGT	yloop1

	LDMFD	r13!,{r4-r11, pc}
fix101:
	@ r7 and r6 are the values, at least one of which has overflowed
	@ r12 = r7 & mask = .s......s......s......
	SUB	r12,r12,r12,LSR #8	@ r12 = ..SSSSSS.SSSSSS.SSSSSS
	ORR	r7, r7, r12		@ r7 |= ..SSSSSS.SSSSSS.SSSSSS
	BIC	r12,r5, r7, LSR #1	@ r12 = .o......o......o......
	ADD	r7, r7, r12,LSR #8	@ r7  = fixed value

	AND	r12, r6, r5		@ r12 = .S......S......S......
	SUB	r12,r12,r12,LSR #8	@ r12 = ..SSSSSS.SSSSSS.SSSSSS
	ORR	r6, r6, r12		@ r6 |= ..SSSSSS.SSSSSS.SSSSSS
	BIC	r12,r5, r6, LSR #1	@ r12 = .o......o......o......
	ADD	r6, r6, r12,LSR #8	@ r6  = fixed value
	B	return101
fix102:
	@ r7 and r6 are the values, at least one of which has overflowed
	@ r12 = r7 & mask = .s......s......s......
	SUB	r12,r12,r12,LSR #8	@ r12 = ..SSSSSS.SSSSSS.SSSSSS
	ORR	r7, r7, r12		@ r7 |= ..SSSSSS.SSSSSS.SSSSSS
	BIC	r12,r5, r7, LSR #1	@ r12 = .o......o......o......
	ADD	r7, r7, r12,LSR #8	@ r7  = fixed value

	AND	r12, r6, r5		@ r12 = .S......S......S......
	SUB	r12,r12,r12,LSR #8	@ r12 = ..SSSSSS..SSSSS.SSSSSS
	ORR	r6, r6, r12		@ r6 |= ..SSSSSS..SSSSS.SSSSSS
	BIC	r12,r5, r6, LSR #1	@ r12 = .o......o......o......
	ADD	r6, r6, r12,LSR #8	@ r6  = fixed value
	B	return102

u_table:
        .word	0x0C440C00
        .word	0x0C341400
        .word	0x0C141C00
        .word	0x0C042400
        .word	0x0BE42C00
        .word	0x0BC43400
        .word	0x0BB43C00
        .word	0x0B944400
        .word	0x0B844C00
        .word	0x0B645400
        .word	0x0B545C00
        .word	0x0B346400
        .word	0x0B246C00
        .word	0x0B047400
        .word	0x0AF47C00
        .word	0x0AD48400
        .word	0x0AC48C00
        .word	0x0AA49400
        .word	0x0A949C00
        .word	0x0A74A400
        .word	0x0A54AC00
        .word	0x0A44B400
        .word	0x0A24BC00
        .word	0x0A14C400
        .word	0x09F4C800
        .word	0x09E4D000
        .word	0x09C4D800
        .word	0x09B4E000
        .word	0x0994E800
        .word	0x0984F000
        .word	0x0964F800
        .word	0x09550000
        .word	0x09350800
        .word	0x09251000
        .word	0x09051800
        .word	0x08E52000
        .word	0x08D52800
        .word	0x08B53000
        .word	0x08A53800
        .word	0x08854000
        .word	0x08754800
        .word	0x08555000
        .word	0x08455800
        .word	0x08256000
        .word	0x08156800
        .word	0x07F57000
        .word	0x07E57800
        .word	0x07C58000
        .word	0x07B58800
        .word	0x07959000
        .word	0x07759800
        .word	0x0765A000
        .word	0x0745A800
        .word	0x0735B000
        .word	0x0715B800
        .word	0x0705C000
        .word	0x06E5C800
        .word	0x06D5D000
        .word	0x06B5D800
        .word	0x06A5E000
        .word	0x0685E800
        .word	0x0675F000
        .word	0x0655F800
        .word	0x06460000
        .word	0x06260800
        .word	0x06161000
        .word	0x05F61400
        .word	0x05D61C00
        .word	0x05C62400
        .word	0x05A62C00
        .word	0x05963400
        .word	0x05763C00
        .word	0x05664400
        .word	0x05464C00
        .word	0x05365400
        .word	0x05165C00
        .word	0x05066400
        .word	0x04E66C00
        .word	0x04D67400
        .word	0x04B67C00
        .word	0x04A68400
        .word	0x04868C00
        .word	0x04669400
        .word	0x04569C00
        .word	0x0436A400
        .word	0x0426AC00
        .word	0x0406B400
        .word	0x03F6BC00
        .word	0x03D6C400
        .word	0x03C6CC00
        .word	0x03A6D400
        .word	0x0396DC00
        .word	0x0376E400
        .word	0x0366EC00
        .word	0x0346F400
        .word	0x0336FC00
        .word	0x03170400
        .word	0x02F70C00
        .word	0x02E71400
        .word	0x02C71C00
        .word	0x02B72400
        .word	0x02972C00
        .word	0x02873400
        .word	0x02673C00
        .word	0x02574400
        .word	0x02374C00
        .word	0x02275400
        .word	0x02075C00
        .word	0x01F76000
        .word	0x01D76800
        .word	0x01C77000
        .word	0x01A77800
        .word	0x01978000
        .word	0x01778800
        .word	0x01579000
        .word	0x01479800
        .word	0x0127A000
        .word	0x0117A800
        .word	0x00F7B000
        .word	0x00E7B800
        .word	0x00C7C000
        .word	0x00B7C800
        .word	0x0097D000
        .word	0x0087D800
        .word	0x0067E000
        .word	0x0057E800
        .word	0x0037F000
        .word	0x0027F800
        .word	0x00080000
        .word	0xFFE80800
        .word	0xFFD81000
        .word	0xFFB81800
        .word	0xFFA82000
        .word	0xFF882800
        .word	0xFF783000
        .word	0xFF583800
        .word	0xFF484000
        .word	0xFF284800
        .word	0xFF185000
        .word	0xFEF85800
        .word	0xFEE86000
        .word	0xFEC86800
        .word	0xFEB87000
        .word	0xFE987800
        .word	0xFE788000
        .word	0xFE688800
        .word	0xFE489000
        .word	0xFE389800
        .word	0xFE18A000
        .word	0xFE08A400
        .word	0xFDE8AC00
        .word	0xFDD8B400
        .word	0xFDB8BC00
        .word	0xFDA8C400
        .word	0xFD88CC00
        .word	0xFD78D400
        .word	0xFD58DC00
        .word	0xFD48E400
        .word	0xFD28EC00
        .word	0xFD18F400
        .word	0xFCF8FC00
        .word	0xFCD90400
        .word	0xFCC90C00
        .word	0xFCA91400
        .word	0xFC991C00
        .word	0xFC792400
        .word	0xFC692C00
        .word	0xFC493400
        .word	0xFC393C00
        .word	0xFC194400
        .word	0xFC094C00
        .word	0xFBE95400
        .word	0xFBD95C00
        .word	0xFBB96400
        .word	0xFBA96C00
        .word	0xFB897400
        .word	0xFB697C00
        .word	0xFB598400
        .word	0xFB398C00
        .word	0xFB299400
        .word	0xFB099C00
        .word	0xFAF9A400
        .word	0xFAD9AC00
        .word	0xFAC9B400
        .word	0xFAA9BC00
        .word	0xFA99C400
        .word	0xFA79CC00
        .word	0xFA69D400
        .word	0xFA49DC00
        .word	0xFA39E400
        .word	0xFA19EC00
        .word	0xF9F9F000
        .word	0xF9E9F800
        .word	0xF9CA0000
        .word	0xF9BA0800
        .word	0xF99A1000
        .word	0xF98A1800
        .word	0xF96A2000
        .word	0xF95A2800
        .word	0xF93A3000
        .word	0xF92A3800
        .word	0xF90A4000
        .word	0xF8FA4800
        .word	0xF8DA5000
        .word	0xF8CA5800
        .word	0xF8AA6000
        .word	0xF89A6800
        .word	0xF87A7000
        .word	0xF85A7800
        .word	0xF84A8000
        .word	0xF82A8800
        .word	0xF81A9000
        .word	0xF7FA9800
        .word	0xF7EAA000
        .word	0xF7CAA800
        .word	0xF7BAB000
        .word	0xF79AB800
        .word	0xF78AC000
        .word	0xF76AC800
        .word	0xF75AD000
        .word	0xF73AD800
        .word	0xF72AE000
        .word	0xF70AE800
        .word	0xF6EAF000
        .word	0xF6DAF800
        .word	0xF6BB0000
        .word	0xF6AB0800
        .word	0xF68B1000
        .word	0xF67B1800
        .word	0xF65B2000
        .word	0xF64B2800
        .word	0xF62B3000
        .word	0xF61B3800
        .word	0xF5FB3C00
        .word	0xF5EB4400
        .word	0xF5CB4C00
        .word	0xF5BB5400
        .word	0xF59B5C00
        .word	0xF57B6400
        .word	0xF56B6C00
        .word	0xF54B7400
        .word	0xF53B7C00
        .word	0xF51B8400
        .word	0xF50B8C00
        .word	0xF4EB9400
        .word	0xF4DB9C00
        .word	0xF4BBA400
        .word	0xF4ABAC00
        .word	0xF48BB400
        .word	0xF47BBC00
        .word	0xF45BC400
        .word	0xF44BCC00
        .word	0xF42BD400
        .word	0xF41BDC00
        .word	0xF3FBE400
        .word	0xF3DBEC00
v_table:
        .word	0x1A000134
        .word	0x19D00135
        .word	0x19A00137
        .word	0x19700139
        .word	0x1930013A
        .word	0x1900013C
        .word	0x18D0013D
        .word	0x1890013F
        .word	0x18600140
        .word	0x18300142
        .word	0x18000144
        .word	0x17C00145
        .word	0x17900147
        .word	0x17600148
        .word	0x1730014A
        .word	0x16F0014C
        .word	0x16C0014D
        .word	0x1690014F
        .word	0x16600150
        .word	0x16200152
        .word	0x15F00154
        .word	0x15C00155
        .word	0x15900157
        .word	0x15500158
        .word	0x1520015A
        .word	0x14F0015C
        .word	0x14C0015D
        .word	0x1480015F
        .word	0x14500160
        .word	0x14200162
        .word	0x13F00164
        .word	0x13B00165
        .word	0x13800167
        .word	0x13500168
        .word	0x1320016A
        .word	0x12E0016C
        .word	0x12B0016D
        .word	0x1280016F
        .word	0x12500170
        .word	0x12100172
        .word	0x11E00174
        .word	0x11B00175
        .word	0x11800177
        .word	0x11400178
        .word	0x1110017A
        .word	0x10E0017C
        .word	0x10B0017D
        .word	0x1070017F
        .word	0x10400180
        .word	0x10100182
        .word	0x0FE00184
        .word	0x0FA00185
        .word	0x0F700187
        .word	0x0F400188
        .word	0x0F10018A
        .word	0x0ED0018B
        .word	0x0EA0018D
        .word	0x0E70018F
        .word	0x0E400190
        .word	0x0E000192
        .word	0x0DD00193
        .word	0x0DA00195
        .word	0x0D700197
        .word	0x0D300198
        .word	0x0D00019A
        .word	0x0CD0019B
        .word	0x0CA0019D
        .word	0x0C60019F
        .word	0x0C3001A0
        .word	0x0C0001A2
        .word	0x0BD001A3
        .word	0x0B9001A5
        .word	0x0B6001A7
        .word	0x0B3001A8
        .word	0x0B0001AA
        .word	0x0AC001AB
        .word	0x0A9001AD
        .word	0x0A6001AF
        .word	0x0A3001B0
        .word	0x09F001B2
        .word	0x09C001B3
        .word	0x099001B5
        .word	0x096001B7
        .word	0x092001B8
        .word	0x08F001BA
        .word	0x08C001BB
        .word	0x089001BD
        .word	0x085001BF
        .word	0x082001C0
        .word	0x07F001C2
        .word	0x07C001C3
        .word	0x078001C5
        .word	0x075001C7
        .word	0x072001C8
        .word	0x06F001CA
        .word	0x06B001CB
        .word	0x068001CD
        .word	0x065001CF
        .word	0x062001D0
        .word	0x05E001D2
        .word	0x05B001D3
        .word	0x058001D5
        .word	0x055001D7
        .word	0x051001D8
        .word	0x04E001DA
        .word	0x04B001DB
        .word	0x048001DD
        .word	0x044001DE
        .word	0x041001E0
        .word	0x03E001E2
        .word	0x03B001E3
        .word	0x037001E5
        .word	0x034001E6
        .word	0x031001E8
        .word	0x02E001EA
        .word	0x02A001EB
        .word	0x027001ED
        .word	0x024001EE
        .word	0x021001F0
        .word	0x01D001F2
        .word	0x01A001F3
        .word	0x017001F5
        .word	0x014001F6
        .word	0x010001F8
        .word	0x00D001FA
        .word	0x00A001FB
        .word	0x007001FD
        .word	0x003001FE
        .word	0x00000200
        .word	0xFFD00202
        .word	0xFF900203
        .word	0xFF600205
        .word	0xFF300206
        .word	0xFF000208
        .word	0xFEC0020A
        .word	0xFE90020B
        .word	0xFE60020D
        .word	0xFE30020E
        .word	0xFDF00210
        .word	0xFDC00212
        .word	0xFD900213
        .word	0xFD600215
        .word	0xFD200216
        .word	0xFCF00218
        .word	0xFCC0021A
        .word	0xFC90021B
        .word	0xFC50021D
        .word	0xFC20021E
        .word	0xFBF00220
        .word	0xFBC00222
        .word	0xFB800223
        .word	0xFB500225
        .word	0xFB200226
        .word	0xFAF00228
        .word	0xFAB00229
        .word	0xFA80022B
        .word	0xFA50022D
        .word	0xFA20022E
        .word	0xF9E00230
        .word	0xF9B00231
        .word	0xF9800233
        .word	0xF9500235
        .word	0xF9100236
        .word	0xF8E00238
        .word	0xF8B00239
        .word	0xF880023B
        .word	0xF840023D
        .word	0xF810023E
        .word	0xF7E00240
        .word	0xF7B00241
        .word	0xF7700243
        .word	0xF7400245
        .word	0xF7100246
        .word	0xF6E00248
        .word	0xF6A00249
        .word	0xF670024B
        .word	0xF640024D
        .word	0xF610024E
        .word	0xF5D00250
        .word	0xF5A00251
        .word	0xF5700253
        .word	0xF5400255
        .word	0xF5000256
        .word	0xF4D00258
        .word	0xF4A00259
        .word	0xF470025B
        .word	0xF430025D
        .word	0xF400025E
        .word	0xF3D00260
        .word	0xF3A00261
        .word	0xF3600263
        .word	0xF3300265
        .word	0xF3000266
        .word	0xF2D00268
        .word	0xF2900269
        .word	0xF260026B
        .word	0xF230026D
        .word	0xF200026E
        .word	0xF1C00270
        .word	0xF1900271
        .word	0xF1600273
        .word	0xF1300275
        .word	0xF0F00276
        .word	0xF0C00278
        .word	0xF0900279
        .word	0xF060027B
        .word	0xF020027C
        .word	0xEFF0027E
        .word	0xEFC00280
        .word	0xEF900281
        .word	0xEF500283
        .word	0xEF200284
        .word	0xEEF00286
        .word	0xEEC00288
        .word	0xEE800289
        .word	0xEE50028B
        .word	0xEE20028C
        .word	0xEDF0028E
        .word	0xEDB00290
        .word	0xED800291
        .word	0xED500293
        .word	0xED200294
        .word	0xECE00296
        .word	0xECB00298
        .word	0xEC800299
        .word	0xEC50029B
        .word	0xEC10029C
        .word	0xEBE0029E
        .word	0xEBB002A0
        .word	0xEB8002A1
        .word	0xEB4002A3
        .word	0xEB1002A4
        .word	0xEAE002A6
        .word	0xEAB002A8
        .word	0xEA7002A9
        .word	0xEA4002AB
        .word	0xEA1002AC
        .word	0xE9E002AE
        .word	0xE9A002B0
        .word	0xE97002B1
        .word	0xE94002B3
        .word	0xE91002B4
        .word	0xE8D002B6
        .word	0xE8A002B8
        .word	0xE87002B9
        .word	0xE84002BB
        .word	0xE80002BC
        .word	0xE7D002BE
        .word	0xE7A002C0
        .word	0xE77002C1
        .word	0xE73002C3
        .word	0xE70002C4
        .word	0xE6D002C6
        .word	0xE6A002C8
        .word	0xE66002C9
        .word	0xE63002CB
y_table:
        .word	0x7FFFFFED
        .word	0x7FFFFFEF
        .word	0x7FFFFFF0
        .word	0x7FFFFFF1
        .word	0x7FFFFFF2
        .word	0x7FFFFFF3
        .word	0x7FFFFFF4
        .word	0x7FFFFFF6
        .word	0x7FFFFFF7
        .word	0x7FFFFFF8
        .word	0x7FFFFFF9
        .word	0x7FFFFFFA
        .word	0x7FFFFFFB
        .word	0x7FFFFFFD
        .word	0x7FFFFFFE
        .word	0x7FFFFFFF
        .word	0x80000000
        .word	0x80500401
        .word	0x80900802
        .word	0x80E00C03
        .word	0x81301405
        .word	0x81701806
        .word	0x81C01C07
        .word	0x82102008
        .word	0x82502409
        .word	0x82A0280A
        .word	0x82F0300C
        .word	0x8330340D
        .word	0x8380380E
        .word	0x83D03C0F
        .word	0x84104010
        .word	0x84604411
        .word	0x84A04C13
        .word	0x84F05014
        .word	0x85405415
        .word	0x85805816
        .word	0x85D05C17
        .word	0x86206018
        .word	0x8660681A
        .word	0x86B06C1B
        .word	0x8700701C
        .word	0x8740741D
        .word	0x8790781E
        .word	0x87E07C1F
        .word	0x88208421
        .word	0x88708822
        .word	0x88C08C23
        .word	0x89009024
        .word	0x89509425
        .word	0x89A09826
        .word	0x89E0A028
        .word	0x8A30A429
        .word	0x8A80A82A
        .word	0x8AC0AC2B
        .word	0x8B10B02C
        .word	0x8B60B42D
        .word	0x8BA0BC2F
        .word	0x8BF0C030
        .word	0x8C40C431
        .word	0x8C80C832
        .word	0x8CD0CC33
        .word	0x8D20D034
        .word	0x8D60D836
        .word	0x8DB0DC37
        .word	0x8DF0E038
        .word	0x8E40E439
        .word	0x8E90E83A
        .word	0x8ED0EC3B
        .word	0x8F20F43D
        .word	0x8F70F83E
        .word	0x8FB0FC3F
        .word	0x90010040
        .word	0x90510441
        .word	0x90910842
        .word	0x90E11044
        .word	0x91311445
        .word	0x91711846
        .word	0x91C11C47
        .word	0x92112048
        .word	0x92512449
        .word	0x92A1284A
        .word	0x92F1304C
        .word	0x9331344D
        .word	0x9381384E
        .word	0x93D13C4F
        .word	0x94114050
        .word	0x94614451
        .word	0x94B14C53
        .word	0x94F15054
        .word	0x95415455
        .word	0x95915856
        .word	0x95D15C57
        .word	0x96216058
        .word	0x9671685A
        .word	0x96B16C5B
        .word	0x9701705C
        .word	0x9741745D
        .word	0x9791785E
        .word	0x97E17C5F
        .word	0x98218461
        .word	0x98718862
        .word	0x98C18C63
        .word	0x99019064
        .word	0x99519465
        .word	0x99A19866
        .word	0x99E1A068
        .word	0x9A31A469
        .word	0x9A81A86A
        .word	0x9AC1AC6B
        .word	0x9B11B06C
        .word	0x9B61B46D
        .word	0x9BA1BC6F
        .word	0x9BF1C070
        .word	0x9C41C471
        .word	0x9C81C872
        .word	0x9CD1CC73
        .word	0x9D21D074
        .word	0x9D61D876
        .word	0x9DB1DC77
        .word	0x9E01E078
        .word	0x9E41E479
        .word	0x9E91E87A
        .word	0x9EE1EC7B
        .word	0x9F21F47D
        .word	0x9F71F87E
        .word	0x9FC1FC7F
        .word	0xA0020080
        .word	0xA0520481
        .word	0xA0920882
        .word	0xA0E21084
        .word	0xA1321485
        .word	0xA1721886
        .word	0xA1C21C87
        .word	0xA2122088
        .word	0xA2522489
        .word	0xA2A22C8B
        .word	0xA2F2308C
        .word	0xA332348D
        .word	0xA382388E
        .word	0xA3D23C8F
        .word	0xA4124090
        .word	0xA4624892
        .word	0xA4B24C93
        .word	0xA4F25094
        .word	0xA5425495
        .word	0xA5925896
        .word	0xA5D25C97
        .word	0xA6226098
        .word	0xA672689A
        .word	0xA6B26C9B
        .word	0xA702709C
        .word	0xA752749D
        .word	0xA792789E
        .word	0xA7E27C9F
        .word	0xA83284A1
        .word	0xA87288A2
        .word	0xA8C28CA3
        .word	0xA90290A4
        .word	0xA95294A5
        .word	0xA9A298A6
        .word	0xA9E2A0A8
        .word	0xAA32A4A9
        .word	0xAA82A8AA
        .word	0xAAC2ACAB
        .word	0xAB12B0AC
        .word	0xAB62B4AD
        .word	0xABA2BCAF
        .word	0xABF2C0B0
        .word	0xAC42C4B1
        .word	0xAC82C8B2
        .word	0xACD2CCB3
        .word	0xAD22D0B4
        .word	0xAD62D8B6
        .word	0xADB2DCB7
        .word	0xAE02E0B8
        .word	0xAE42E4B9
        .word	0xAE92E8BA
        .word	0xAEE2ECBB
        .word	0xAF22F4BD
        .word	0xAF72F8BE
        .word	0xAFC2FCBF
        .word	0xB00300C0
        .word	0xB05304C1
        .word	0xB0A308C2
        .word	0xB0E310C4
        .word	0xB13314C5
        .word	0xB18318C6
        .word	0xB1C31CC7
        .word	0xB21320C8
        .word	0xB25324C9
        .word	0xB2A32CCB
        .word	0xB2F330CC
        .word	0xB33334CD
        .word	0xB38338CE
        .word	0xB3D33CCF
        .word	0xB41340D0
        .word	0xB46348D2
        .word	0xB4B34CD3
        .word	0xB4F350D4
        .word	0xB54354D5
        .word	0xB59358D6
        .word	0xB5D35CD7
        .word	0xB62364D9
        .word	0xB67368DA
        .word	0xB6B36CDB
        .word	0xB70370DC
        .word	0xB75374DD
        .word	0xB79378DE
        .word	0xB7E37CDF
        .word	0xB83384E1
        .word	0xB87388E2
        .word	0xB8C38CE3
        .word	0xB91390E4
        .word	0xB95394E5
        .word	0xB9A398E6
        .word	0xB9F3A0E8
        .word	0xBA33A4E9
        .word	0xBA83A8EA
        .word	0xBAD3ACEB
        .word	0xBB13B0EC
        .word	0xBB63B4ED
        .word	0xBBA3BCEF
        .word	0xBBF3C0F0
        .word	0xBC43C4F1
        .word	0xBC83C8F2
        .word	0xBCD3CCF3
        .word	0xBD23D0F4
        .word	0xBD63D8F6
        .word	0xBDB3DCF7
        .word	0xBE03E0F8
        .word	0xBE43E4F9
        .word	0xBE93E8FA
        .word	0xBEE3ECFB
        .word	0xBF23F4FD
        .word	0xBF73F8FE
        .word	0xBFC3FCFF
        .word	0xC0040100
        .word	0xC0540501
        .word	0xC0A40902
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104
        .word	0xC0E41104

	@ END
