/* YUV-> RGB conversion code.
 *
 * Copyright (C) 2011 Robin Watts (robin@wss.co.uk) for Pinknoise
 * Productions Ltd.
 *
 * Licensed under the BSD license. See 'COPYING' for details of
 * (non-)warranty.
 *
 */

#ifndef YUV2RGB_H

#define YUV2RGB_H

/* Define these to something appropriate in your build */
/*typedef unsigned int   unsigned int;
typedef signed   int   signed int;
typedef unsigned short uint16_t;
typedef unsigned char  unsigned char;*/

/* Define these to something appropriate in your build */
typedef unsigned int   uint32_t;
typedef signed   int   int32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

extern const unsigned int yuv2rgb565_table[];
extern const unsigned int yuv2bgr565_table[];

#ifdef __cplusplus
extern "C" {
#endif
void yuv420_2_rgb565(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv422_2_rgb565(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv444_2_rgb565(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv420_2_rgb888(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv422_2_rgb888(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv444_2_rgb888(unsigned char  *dst_ptr,
               const unsigned char  *y_ptr,
               const unsigned char  *u_ptr,
               const unsigned char  *v_ptr,
                     signed int   width,
                     signed int   height,
                     signed int   y_span,
                     signed int   uv_span,
                     signed int   dst_span,
               const unsigned int *tables,
                     signed int   dither);

void yuv420_2_rgb8888(unsigned char  *dst_ptr,
                const unsigned char  *y_ptr,
                const unsigned char  *u_ptr,
                const unsigned char  *v_ptr,
                      signed int   width,
                      signed int   height,
                      signed int   y_span,
                      signed int   uv_span,
                      signed int   dst_span,
                const unsigned int *tables,
                      signed int   dither);

void yuv422_2_rgb8888(unsigned char  *dst_ptr,
                const unsigned char  *y_ptr,
                const unsigned char  *u_ptr,
                const unsigned char  *v_ptr,
                      signed int   width,
                      signed int   height,
                      signed int   y_span,
                      signed int   uv_span,
                      signed int   dst_span,
                const unsigned int *tables,
                      signed int   dither);

void yuv444_2_rgb8888(unsigned char  *dst_ptr,
                const unsigned char  *y_ptr,
                const unsigned char  *u_ptr,
                const unsigned char  *v_ptr,
                      signed int   width,
                      signed int   height,
                      signed int   y_span,
                      signed int   uv_span,
                      signed int   dst_span,
                const unsigned int *tables,
                      signed int   dither);

#ifdef __cplusplus
}
#endif


#endif /* YUV2RGB_H */
