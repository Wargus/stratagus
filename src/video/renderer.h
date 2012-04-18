#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <assert.h>

struct CPrimitives {
	virtual void DrawPixel(Uint32 color, int x, int y) = 0;
	virtual void DrawTransPixel(Uint32 color, int x, int y,
								unsigned char alpha) = 0;

	virtual void DrawLine(Uint32 color, int sx, int sy, int dx, int dy) = 0;
	virtual void DrawTransLine(Uint32 color, int sx, int sy,
							   int dx, int dy, unsigned char alpha) = 0;

	virtual void DrawRectangle(Uint32 color, int x, int y, int w, int h) = 0;
	virtual void DrawTransRectangle(Uint32 color, int x, int y,
									int w, int h, unsigned char alpha) = 0;

	virtual void FillTransRectangle(Uint32 color, int x, int y,
									int w, int h, unsigned char alpha) = 0;

	virtual void DrawCircle(Uint32 color, int x, int y, int r) = 0;
	virtual void DrawTransCircle(Uint32 color, int x, int y,
								 int r, unsigned char alpha) = 0;

	virtual void FillCircle(Uint32 color, int x, int y, int r) = 0;
	virtual void FillTransCircle(Uint32 color, int x, int y,
								 int r, unsigned char alpha) = 0;
};

namespace DRAW
{
template <const int BPP>
static inline Uint32 GetPixel(const void *const pixels, unsigned int index)
{
	if (BPP == 1) {
		return (((Uint8 *)pixels)[index]);
	} else if (BPP == 2) {
		return (((Uint16 *)pixels)[index]);
	} else if (BPP == 4) {
		return (((Uint32 *)pixels)[index]);
	} else {
		assert(0);
	}
	return 0;
};// __attribute__ ((nothrow,nonnull (1)));

template <const int BPP>
static inline void PutPixel(void *const pixels,
							unsigned int index, Uint32 color)
{
	if (BPP == 1) {
		((Uint8 *)pixels)[index] = color;
	} else if (BPP == 2) {
		((Uint16 *)pixels)[index] = color;
	} else if (BPP == 4) {
		((Uint32 *)pixels)[index] = color;
	} else {
		assert(0);
	}
};// __attribute__ ((nothrow,nonnull (1)));

template <const int BPP>
static inline void
PutPixelDouble(void *pixels, const unsigned int index, Uint32 color)
{
	if (BPP == 1) {
		color &= 0xFF;
		color |= color << 8;
		*((Uint16 *)(((Uint8 *)pixels) + index)) = color;
	} else if (BPP == 2) {
		color &= 0xFFFF;
		color |= color << 16;
		*((Uint32 *)(((Uint16 *)pixels) + index)) = color;
	} else if (BPP == 4) {
		Uint32 *ptr = ((Uint32 *)pixels) + index;
#ifdef __x86_64__
		Uint64 tmp = color;
		tmp <<= 32;
		//tmp |= color;
		*((Uint64 *)ptr) = tmp | color;
#else
		*ptr++ = color;
		*ptr = color;
#endif
	} else {
		assert(0);
	}
};// __attribute__ ((nothrow,nonnull (1)));

template <const int BPP>
static inline void
PutPixelQuatro(void *pixels, unsigned int index, Uint32 color)
{
	if (BPP == 1) {
		color &= 0xFF;
		color |= color << 8;
		color |= color << 16;
		*((Uint32 *)(((Uint8 *)pixels) + index)) = color;
	} else if (BPP == 2) {
		Uint32 *ptr = (Uint32 *)(((Uint16 *)pixels) + index);
		color &= 0xFFFF;
		color |= color << 16;
#ifdef __x86_64__
		Uint64 tmp = color;
		tmp <<= 32;
		//tmp |= color;
		*((Uint64 *)ptr) = tmp | color;
#else
		*ptr++ = color;
		*ptr = color;
#endif
	} else if (BPP == 4) {
		Uint32 *ptr = ((Uint32 *)pixels) + index;
#ifdef __x86_64__
		Uint64 tmp = color;
		tmp <<= 32;
		tmp |= color;
		*((Uint64 *)ptr) = tmp;
		*((Uint64 *)(ptr + 2)) = tmp;
#else
		*ptr++ = color;
		*ptr++ = color;
		*ptr++ = color;
		*ptr = color;
#endif
	} else {
		assert(0);
	}
};// __attribute__ ((nothrow,nonnull (1)));

template <const int BPP>
static inline void DrawHLine(void *pixels, unsigned int index,
							 int width, Uint32 color)
{
#if 0
	//if(width < 1) return;
	do {
		PutPixel(pixels, index, color);
		index++;
	} while (--width);
#else
	if (BPP == 1) {
		memset((void *)(((Uint8 *)pixels) + index), color, width);
	} else if (BPP == 2) {
#ifdef __x86_64__
		switch (((uintptr_t)pixels) & 6) {
			case 6:
				PutPixel<BPP>(pixels, index, color);
				index++;
				--width;
			case 4:
				PutPixel<BPP>(pixels, index, color);
				index++;
				--width;
			case 2:
				PutPixel<BPP>(pixels, index, color);
				index++;
				--width;
			default:
				break;
		}
#else
		if (((uintptr_t)pixels) & BPP) {
			PutPixel<BPP>(pixels, index, color);
			index++;
			--width;
		}
#endif
	}
#ifdef __x86_64__
	else if (BPP == 4 && ((uintptr_t)pixels) & BPP) {
		PutPixel<BPP>(pixels, index, color);
		index++;
		--width;
	}
#endif
	while (width > 3) {
		PutPixelQuatro<BPP>(pixels, index, color);
		index += 4;
		width -= 4;
	};
	switch (width & 3) {
		case 3:
			PutPixel<BPP>(pixels, index, color);
			index++;
			//--width;
		case 2:
			PutPixel<BPP>(pixels, index, color);
			index++;
			//--width;
		case 1:
			PutPixel<BPP>(pixels, index, color);
			index++;
			//--width;
		default:
			break;
	}

#endif
};

};

//RGB565 -> MASK = 0xf7de
//RGB555 -> MASK = 0xfbde
//RGB888 -> MASK = 0
template <const int BPP, const int MASK>
class CRenderer : public CPrimitives
{

	static inline void PutTransPixel(void *pixels, unsigned int index,
									 Uint32 color, unsigned int alpha) {
		unsigned int dp;
		if (BPP == 2) {
			Uint16 *p = (((Uint16 *)pixels) + index);
			// Loses precision for speed
			alpha >>= 3;
			if (MASK == 0xf7de) {
				/* RGB565 */
				color = (((color << 16) | color) & 0x07E0F81F);
				dp = *p;
				dp = ((dp << 16) | dp) & 0x07E0F81F;
				dp += (((color - dp) * alpha) >> 5);
				dp &= 0x07E0F81F;
			} else {
				if (MASK == 0xfbde) {
					/* RGB555 */
					color = (((color << 16) | color) & 0x03e07c1f);
					dp = *p;
					dp = ((dp << 16) | dp) & 0x03e07c1f;
					dp += (((color - dp) * alpha) >> 5);
					dp &= 0x03e07c1f;
				} else {
					assert(0);
				}
			}
			*p = (dp >> 16) | dp;
		} else if (BPP == 4) {
			/* RGB888(8) */
			Uint32 *p = (((Uint32 *)pixels) + index);
			unsigned int sp2 = (color & 0xFF00FF00) >> 8;
			unsigned int dp2;

			color &= 0x00FF00FF;

			dp = *p;
			dp2 = (dp & 0xFF00FF00) >> 8;
			dp &= 0x00FF00FF;

			dp += (((color - dp) * alpha) >> 8);
			dp &= 0x00FF00FF;
			dp2 += (((sp2 - dp2) * alpha) >> 8);
			dp2 &= 0x00FF00FF;
			*p = (dp | (dp2 << 8));
		} else {
			assert(0);
		}
	} // __attribute__ ((nothrow,nonnull (1)));

	static inline void PutTransPixelDouble(void *pixels, unsigned int index,
										   Uint32 color, unsigned int alpha) {

		if (BPP == 2) {
			/*
			#ifdef __x86_64__
						//FIXME
						unsigned int dp;
						Uint16 *p = (((Uint16 *)pixels) + index);

			#else
			*/
			PutTransPixel(pixels, index, color, alpha);
			PutTransPixel(pixels, index + 1, color, alpha);
			//#endif
		} else if (BPP == 4) {
#ifdef __x86_64__
			Uint64 *const p = (Uint64 *)(((Uint32 *)pixels) + index);
			const Uint64 A = alpha;
			Uint64 src0 = color;
			Uint64 src1 = (src0 | (src0 << 32));
			Uint64 dst0 = *p;
			Uint64 dst1 = (dst0 >> 8);

			src0 = src1;
			src1 >>= 8;

			src0 &= 0x00FF00FF00FF00FF;
			src1 &= 0x00FF00FF00FF00FF;

			dst0 &= 0x00FF00FF00FF00FF;
			dst1 &= 0x00FF00FF00FF00FF;

			dst0 += ((src0 - dst0) * A >> 8);
			dst0 &= 0x00FF00FF00FF00FF;

			dst1 += ((src1 - dst1) * A >> 8);
			dst1 &= 0x00FF00FF00FF00FF;

			*p = dst0 | (dst1 << 8)
#else
			Uint32 * p = (((Uint32 *)pixels) + index);
			/*
			 *	FIXME:
			 *	Two Pixels Blend for litle endian and
			 *	big endian may be broken.
			 */
			unsigned int d1, s1 = color & 0xff00ff;
			unsigned int dp = *p;
			d1 = dp & 0xff00ff;

			color &= 0xff00;
			color = (color >> 8) | (color << 8);

			d1 += (s1 - d1) * alpha >> 8;
			d1 &= 0xff00ff;

			dp = ((dp & 0xff00) >> 8) |
				 ((p[1] & 0xff00) << 8);

			dp += (color - dp) * alpha >> 8;
			dp &= 0x00ff00ff;

			*p++ = d1 | ((dp << 8) & 0xff00) | 0xff000000;

			d1 = *p;
			d1 &= 0xff00ff;
			d1 += (s1 - d1) * alpha >> 8;
			d1 &= 0xff00ff;

			*p = d1 | ((dp >> 8) & 0xff00) | 0xff000000;
#endif
		} else {
			assert(0);
		}
	} // __attribute__ ((nothrow,nonnull (1)));


	static inline void PutTransPixel128(void *pixels, unsigned int index, Uint32 color) {
		if (BPP == 2) {
			/* blend a single 16 bit pixel at 50% */
#define BLEND16_50(d, s, mask)						\
	((((s & mask) + (d & mask)) >> 1) + (s & d & (~mask & 0xffff)))

			Uint16 *p = (((Uint16 *)pixels) + index);
			Uint16 d = *p;
			Uint16 s = color & 0xFFFF; // I hope that caler secure it;

			*p = BLEND16_50(d, s, MASK);

#undef BLEND16_50
		} else if (BPP == 4) {
			Uint32 *p = (((Uint32 *)pixels) + index);
			unsigned int d = *p;

			*p = ((((color & 0x00fefefe) + (d & 0x00fefefe)) >> 1)
				  + (color & d & 0x00010101)) | 0xff000000;

		} else {
			assert(0);
		}
	};

	static inline void PutTransPixel128Double(void *pixels, const unsigned int index,
											  const Uint32 color) {
		if (BPP == 2) {
			/* blend two 16 bit pixels at 50% */
#define BLEND2x16_50(d, s, mask)					     \
	(((s & (mask | mask << 16)) >> 1) + ((d & (mask | mask << 16)) >> 1) \
	 + (s & d & (~(mask | mask << 16))))

			Uint32 *p = (Uint32 *)(((Uint16 *)pixels) + index);
			Uint32 d = *p;
			const Uint32 s = (color & 0xFFFF) | color << 16; // I hope that caler secure it;

			*p = BLEND2x16_50(d, s, MASK);

#undef BLEND2x16_50
		} else if (BPP == 4) {
			Uint32 *p = (((Uint32 *)pixels) + index);
#ifdef __x86_64__
			unsigned long long int d = *(unsigned long long int *)p;
			unsigned long long int s, c = color;

			s = c | c << 32;
			c = s & 0x00fefefe00fefefe;

			*(unsigned long long int *)p =
				(((c + (d & 0x00fefefe00fefefe)) >> 1)
				 + (s & d & 0x0001010100010101)) |
				0xff000000ff000000;
#else
			Uint32 d = *p;
			const Uint32 c = color & 0x00fefefe;
			*p++ = (((c + (d & 0x00fefefe)) >> 1)
					+ (color & d & 0x00010101)) | 0xff000000;
			d = *p;
			*p = (((c + (d & 0x00fefefe)) >> 1)
				  + (color & d & 0x00010101)) | 0xff000000;
#endif
		} else {
			assert(0);
		}
	} // __attribute__ ((nothrow,nonnull (1)));

	static void DrawVLine(void *pixels, const unsigned int pitch,
						  unsigned int index, int height, Uint32 color) {
		if (height < 1) { return; }
		do {
			DRAW::PutPixel<BPP>(pixels, index, color);
			index += pitch;
		} while (--height);
	} // __attribute__ ((nothrow,nonnull (1)));

	static void DrawTransVLine(void *pixels, const unsigned int pitch,
							   unsigned int index, int height, Uint32 color, unsigned int alpha) {
		if (height < 1) { return; }
		if (alpha == 128) {
			do {
				PutTransPixel128(pixels, index, color);
				index += pitch;
			} while (--height);
		} else {
			do {
				PutTransPixel(pixels, index, color, alpha);
				index += pitch;
			} while (--height);
		}
	} // __attribute__ ((nothrow,nonnull (1)));

	static inline void DrawTransHLine128(void *pixels,
										 unsigned int index, int width, Uint32 color) {
#ifdef __x86_64__
		//FIXME: this may not work on 16 bpp
		if (((uintptr_t)pixels) & BPP) {
#else
		if (width & 1) {
#endif
			PutTransPixel128(pixels, index, color);
			--width;
			index++;
		}
		while (width > 1) {
			PutTransPixel128Double(pixels, index, color);
			index += 2;
			width -= 2;
		};
#ifdef __x86_64__
		if (width) { PutTransPixel128(pixels, index, color); }
#endif

	} // __attribute__ ((nothrow,nonnull (1)));


	static inline void DrawTransHLineNon128(void *pixels,
											unsigned int index, int width, Uint32 color, unsigned int alpha) {
#ifdef __x86_64__
		if (((uintptr_t)pixels) & BPP) {
#else
		if (width & 1) {
#endif
			PutTransPixel(pixels, index, color, alpha);
			--width;
			index++;
		}
		while (width > 1) {
			PutTransPixelDouble(pixels, index, color, alpha);
			index += 2;
			width -= 2;
		};
#ifdef __x86_64__
		if (width) { PutTransPixel(pixels, index, color, alpha); }
#endif
	} // __attribute__ ((nothrow,nonnull (1)));

	static inline void DrawTransHLine(void *pixels,
									  unsigned int index, int width, Uint32 color, unsigned int alpha) {
		//if(width < 1) return;
		if (alpha == 128) {
			DrawTransHLine128(pixels, index, width, color);
		} else {
			DrawTransHLineNon128(pixels, index, width, color, alpha);
		}
	};

	void DrawPixel(Uint32 color, int x, int y) {
		unsigned int index =  TheScreen->pitch / BPP;
		index *= y;
		index += x;
		DRAW::PutPixel<BPP>(TheScreen->pixels, index, color);
	};

	void DrawTransPixel(Uint32 color, int x, int y,  unsigned char alpha) {
		unsigned int index =  TheScreen->pitch / BPP;
		index *= y;
		index += x;
		if (alpha == 128) {
			PutTransPixel128(TheScreen->pixels, index, color);
		} else {
			PutTransPixel(TheScreen->pixels, index, color, alpha);
		}
	};

	void DrawLine(Uint32 color, int sx, int sy, int dx, int dy) {
		unsigned int index;
		const unsigned int pitch = TheScreen->pitch / BPP;

		if (sx == dx) {
			unsigned int len = 1;
			if (sy < dy) {
				index = sx + sy * pitch;
				len += dy - sy;
			} else {
				index = dx + dy * pitch;
				len += sy - dy;
			}
			DrawVLine(TheScreen->pixels, pitch, index, len, color);
			return;
		}

		if (sy == dy) {
			unsigned int len = 1;
			if (sx < dx) {
				index = sx + sy * pitch;
				len += dx - sx;
			} else {
				index = dx + dy * pitch;
				len += sx - dx;
			}
			DRAW::DrawHLine<BPP>(TheScreen->pixels, index, len, color);
			return;
		}

		// exchange coordinates
		if (sy > dy) {
			int t;
			t = dx;
			dx = sx;
			sx = t;
			t = dy;
			dy = sy;
			sy = t;
		}

		int ylen = dy - sy;
		int incr;
		int xlen;

		if (sx > dx) {
			xlen = sx - dx;
			incr = -1;
		} else {
			xlen = dx - sx;
			incr = 1;
		}

		int y = sy;
		int x = sx;

		if (xlen > ylen) {
			int p;

			if (sx > dx) {
				int t;
				t = sx;
				sx = dx;
				dx = t;
				y = dy;
			}

			p = (ylen << 1) - xlen;

			index = y * pitch;
			incr *= pitch;
			for (x = sx; x < dx; ++x) {
				DRAW::PutPixel<BPP>(TheScreen->pixels, x + index, color);
				if (p >= 0) {
					//y += incr;
					index += incr;
					p += (ylen - xlen) << 1;
				} else {
					p += (ylen << 1);
				}
			}
			return;
		}

		if (ylen > xlen) {
			int p;

			p = (xlen << 1) - ylen;
			index = sy * pitch;
			for (y = sy; y < dy; ++y) {
				DRAW::PutPixel<BPP>(TheScreen->pixels, x + index, color);
				if (p >= 0) {
					x += incr;
					p += (xlen - ylen) << 1;
				} else {
					p += (xlen << 1);
				}
				index += pitch;
			}

			return;
		}

		// Draw a diagonal line
		if (ylen == xlen) {
			index = y * pitch;
			while (y != dy) {
				DRAW::PutPixel<BPP>(TheScreen->pixels, x + index, color);
				x += incr;
				++y;
				index += pitch;
			}
		}
	};

	void DrawTransLine(Uint32 color, int sx, int sy,
					   int dx, int dy, unsigned char alpha) {
		DrawLine(color, sx, sy, dx, dy);
	}


	void DrawRectangle(Uint32 color, int x, int y, int w, int h) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		unsigned int index = y * pitch;
		unsigned int y_offset = (h - 1) * pitch;
		PutTransPixel128(TheScreen->pixels, x + index, color);
		DRAW::DrawHLine<BPP>(TheScreen->pixels, x + index + 1, w - 2, color); ///(x,y,w)
		PutTransPixel128(TheScreen->pixels, x + index + w - 1, color);

		PutTransPixel128(TheScreen->pixels, x + index + y_offset, color);
		DRAW::DrawHLine<BPP>(TheScreen->pixels, x + index + y_offset + 1, w - 2, color); // (x, y + h - 1, w)
		PutTransPixel128(TheScreen->pixels, x + index + y_offset + w - 1, color);

		DrawVLine(TheScreen->pixels, pitch, x + index + pitch,
				  h - 2, color); //(x, y + 1, h - 2)
		DrawVLine(TheScreen->pixels, pitch, x + index + w - 1 + pitch,
				  h - 2, color);	//x + w - 1, y + 1, h - 2
	};

	void DrawTransRectangle(Uint32 color, int x, int y,
							int w, int h, unsigned char alpha) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		unsigned int index = y * pitch;
		unsigned int y_offset = (h - 1) * pitch;
		//unsigned int a = 255 - alpha;

		PutTransPixel(TheScreen->pixels, x + index, color, alpha / 2);
		DrawTransHLine(TheScreen->pixels, x + index + 1, w - 2, color, alpha); ///(x,y,w)
		PutTransPixel(TheScreen->pixels, x + index + w - 1, color, alpha / 2);

		PutTransPixel(TheScreen->pixels, x + index + y_offset, color, alpha / 2);
		DrawTransHLine(TheScreen->pixels, x + index + y_offset + 1,
					   w - 2, color, alpha); // (x, y + h - 1, w)
		PutTransPixel(TheScreen->pixels,
					  x + index + y_offset + w - 1, color, alpha / 2);

		DrawTransVLine(TheScreen->pixels, pitch, x + index + pitch,
					   h - 2, color, alpha); //(x, y + 1, h - 2)
		DrawTransVLine(TheScreen->pixels, pitch, x + index + w - 1 + pitch,
					   h - 2, color, alpha); //x + w - 1, y + 1, h - 2
	};

	void FillTransRectangle(Uint32 color, int x, int y,
							int w, int h, unsigned char alpha) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		unsigned int index = y * pitch;
		if (alpha == 128) {
			do {
				DrawTransHLine128(TheScreen->pixels, x + index, w, color);
				index += pitch;
			} while (--h);
		} else {
			do {
				DrawTransHLineNon128(TheScreen->pixels, x + index, w, color, alpha);
				index += pitch;
			} while (--h);
		}
	};

	void DrawCircle(Uint32 color, int x, int y, int r) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		int p = 1 - r;
		int px = 0;
		int py = r;

		for (; px <= py + 1; ++px) {
			unsigned int index_plus = (y + py) * pitch;
			unsigned int index_minus = (y - py) * pitch;

			DRAW::PutPixel<BPP>(TheScreen->pixels, x + px + index_plus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x + px + index_minus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x - px + index_plus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x - px + index_minus, color);

			index_plus = (y + px) * pitch;
			index_minus = (y - px) * pitch;

			DRAW::PutPixel<BPP>(TheScreen->pixels, x + py + index_plus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x + py + index_minus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x - py + index_plus, color);
			DRAW::PutPixel<BPP>(TheScreen->pixels, x - py + index_minus, color);

			if (p < 0) {
				p += 2 * px + 3;
			} else {
				p += 2 * (px - py) + 5;
				py -= 1;
			}
		}
	};

	void DrawTransCircle(Uint32 color, int x, int y,
						 int r, unsigned char alpha) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		int p = 1 - r;
		int px = 0;
		int py = r;

		for (; px <= py + 1; ++px) {
			unsigned int index_plus = (y + py) * pitch;
			unsigned int index_minus = (y - py) * pitch;

			PutTransPixel(TheScreen->pixels, x + px + index_plus, color, alpha);
			PutTransPixel(TheScreen->pixels, x + px + index_minus, color, alpha);
			PutTransPixel(TheScreen->pixels, x - px + index_plus, color, alpha);
			PutTransPixel(TheScreen->pixels, x - px + index_minus, color, alpha);

			index_plus = (y + px) * pitch;
			index_minus = (y - px) * pitch;

			PutTransPixel(TheScreen->pixels, x + py + index_plus, color, alpha);
			PutTransPixel(TheScreen->pixels, x + py + index_minus, color, alpha);
			PutTransPixel(TheScreen->pixels, x - py + index_plus, color, alpha);
			PutTransPixel(TheScreen->pixels, x - py + index_minus, color, alpha);

			if (p < 0) {
				p += 2 * px + 3;
			} else {
				p += 2 * (px - py) + 5;
				py -= 1;
			}
		}
	};

	void FillCircle(Uint32 color, int x, int y, int r) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		int p = 1 - r;
		int px = 0;
		int py = r;

		for (; px <= py; ++px) {
			//FIXME: Change it to DrawHLine for speed.
			unsigned int y_index = y * pitch;
			unsigned int py_index = py * pitch;

			// Fill up the middle half of the circle
			DrawVLine(TheScreen->pixels, pitch,
					  x + px + y_index,
					  py + 1, color);
			DrawVLine(TheScreen->pixels, pitch,
					  x + px + (y_index - py_index),
					  py, color);

			if (px) {
				DrawVLine(TheScreen->pixels, pitch,
						  x - px + y_index,
						  py + 1, color);
				DrawVLine(TheScreen->pixels, pitch,
						  x - px + (y_index - py_index),
						  py, color);
			}

			if (p < 0) {
				p += 2 * px + 3;
			} else {
				p += 2 * (px - py) + 5;
				py -= 1;
				// Fill up the left/right half of the circle
				if (py >= px) {
					unsigned int px_index = px * pitch;
					DrawVLine(TheScreen->pixels, pitch,
							  x + py + 1 + y_index,
							  px + 1, color);
					DrawVLine(TheScreen->pixels, pitch,
							  x + py + 1 + (y_index - px_index),
							  px, color);
					DrawVLine(TheScreen->pixels, pitch,
							  x - py - 1 + y_index,
							  px + 1, color);
					DrawVLine(TheScreen->pixels, pitch,
							  x - py - 1 + (y_index - px_index),
							  px, color);
				}
			}
		}
	};

	void FillTransCircle(Uint32 color, int x, int y,
						 int r, unsigned char alpha) {
		const unsigned int pitch = TheScreen->pitch / BPP;
		int p = 1 - r;
		int px = 0;
		int py = r;

		for (; px <= py; ++px) {
			//FIXME: Change it to DrawTransHLine for speed.
			unsigned int y_index = y * pitch;
			unsigned int py_index = py * pitch;

			// Fill up the middle half of the circle
			DrawTransVLine(TheScreen->pixels, pitch,
						   x + px + y_index,
						   py + 1, color, alpha);
			DrawTransVLine(TheScreen->pixels, pitch,
						   x + px + (y_index - py_index),
						   py, color, alpha);

			if (px) {
				DrawTransVLine(TheScreen->pixels, pitch,
							   x - px + y_index,
							   py + 1, color, alpha);
				DrawTransVLine(TheScreen->pixels, pitch,
							   x - px + (y_index - py_index),
							   py, color, alpha);
			}

			if (p < 0) {
				p += 2 * px + 3;
			} else {
				p += 2 * (px - py) + 5;
				py -= 1;
				// Fill up the left/right half of the circle
				if (py >= px) {
					unsigned int px_index = px * pitch;
					DrawTransVLine(TheScreen->pixels, pitch,
								   x + py + 1 + y_index,
								   px + 1, color, alpha);
					DrawTransVLine(TheScreen->pixels, pitch,
								   x + py + 1 + (y_index - px_index),
								   px, color, alpha);
					DrawTransVLine(TheScreen->pixels, pitch,
								   x - py - 1 + y_index,
								   px + 1, color, alpha);
					DrawTransVLine(TheScreen->pixels, pitch,
								   x - py - 1 + (y_index - px_index),
								   px, color, alpha);
				}
			}
		}
	};

};

typedef CRenderer<2, 0xfbde> Primitive16_555_t;
typedef CRenderer<2, 0xf7de> Primitive16_565_t;
typedef CRenderer<4, 0> Primitive32_t;

#endif

