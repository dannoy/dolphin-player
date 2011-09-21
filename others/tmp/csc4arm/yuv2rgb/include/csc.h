#ifndef CSC_H_INCLUDED
#define CSC_H_INCLUDED

#if __cplusplus
extern "C" {
#endif

#define LIBCSC_ARMV5T_VERSION	"libcsc-armv5t 0.01"

/* RGB to YUV conversions */

/**
 * @param src	the pointer to the src rgb line buffer. 
 * @param dst	the pointer to the dst YUV line buffer.
 * @param width width of the line. must be multiply of 2.
 */
extern void __ABGR8888toYUV422(const unsigned char *src, unsigned char *dst, unsigned int width);
extern void __ARGB8888toYUV422(const unsigned char *src, unsigned char *dst, unsigned int width);
extern void __BGR888toYUV422(const unsigned char *src, unsigned char *dst, unsigned int width);
extern void __RGB888toYUV422(const unsigned char *src, unsigned char *dst, unsigned int width);
extern void __RGBA8888toYUV422(const unsigned char *src, unsigned char *dst, unsigned int width);

extern void __BGR888toYUV422_C(const unsigned char *src, unsigned char *dst, unsigned int width);
#if __cplusplus
}
#endif


#endif // CSC_H_INCLUDED
