#define PRECISION_SHIFT     13

#include "rgb_to_yuv_constants.h"

void __BGR888toYUV422_C(const unsigned char *src, unsigned char *dst, int width)
{
	int r, g, b, y, u, v;
	while (width--) {
		b = *src++;
		g = *src++;
		r = *src++;

		y = (r * C_0_29891 + g * C_0_58661 + b * C_0_11448) >> PRECISION_SHIFT;
		u = ((r * -C_0_16874 + g * -C_0_33126 + b * C_0_50000) >> PRECISION_SHIFT) + 128;
		v = ((r * C_0_50000 + g * -C_0_41869 + b * -C_0_08131) >> PRECISION_SHIFT) + 128;
		*dst++ = u;
		*dst++ = y;
		*dst++ = v;

		b = *src++;
		g = *src++;
		r = *src++;

		y = (r * C_0_29891 + g * C_0_58661 + b * C_0_11448) >> PRECISION_SHIFT;
		*dst++ = y;
	}
}

