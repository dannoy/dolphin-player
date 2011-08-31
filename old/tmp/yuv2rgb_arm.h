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

extern const short RGBY[256][4];
extern const short RGBU[256][4];
extern const short RGBV[256][4];

/*
 *
 * mb_YUV420_to_RGB32
 * Performs YUV420 to RGB conversion, 32bit depth color
 * Constraints:
 * The frame must be an even width and height, otherwise the result is unpredictable
 *
 * Input:
 *  int size: The size of the RGB frame, packed as (width << 16 | height)
 *  int lumaStep: the width of a luminance line, this is needed because they are often
 *                wider than the actual frame when decoding videos.
 *  char *yuv[]: array of pointers to the Y, U and V planes
 *  char *dest: pointer to the output RGB buffer. It must be allocated and at least
 *              width * height * 4 bytes long.
 *
 * Output:
 *  char *dest: On output, contains the decoded frame
 *
 * Return Value:
 *  void
 */

void mb_YUV420_to_RGB32(int size, int lumaStep, char *yuv[], char *dest);
