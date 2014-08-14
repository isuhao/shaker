#ifndef __GFX_BASIC_HPP__
#define __GFX_BASIC_HPP__

#include <ppapi/cpp/image_data.h>
#include <stdint.h>

typedef uint32_t size_t;

namespace gfx
{
	inline uint32_t RGB24(uint8_t r, uint8_t g, uint8_t b)
	{
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
		if (format == PP_IMAGEDATAFORMAT_BGRA_PREMUL)
		{
			return (0xFF << 24) | (r << 16) | (g << 8) | b;
		}
		else {
			return (0xFF << 24) | (b << 16) | (g << 8) | r;
		}
	}

	inline uint32_t ARGB32(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
	{
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
		if (format == PP_IMAGEDATAFORMAT_BGRA_PREMUL)
		{
			return (a << 24) | (r << 16) | (g << 8) | b;
		}
		else {
			return (a << 24) | (b << 16) | (g << 8) | r;
		}
	}

	inline uint8_t clamp(int i)
	{
		if (i < 0) return 0;
		if (i > 255) return 255;
		return (uint8_t)i;
	}

	inline uint8_t blend(int alpha, int over, int under)
	{
		return clamp(((int)over * alpha + under * (255 - alpha)) / 255);
	}

	struct PlanarYUV420
	{
		size_t halfWidth, halfHeight;
		size_t Yplane;
		size_t UVplane;
		const uint8_t* in;
		uint32_t* out;

		struct Pixel4
		{
			const uint8_t* Y0;
			const uint8_t* Y1;
			const uint8_t* U;
			const uint8_t* V;
			uint32_t* RGB0;
			uint32_t* RGB1;

			Pixel4(const uint8_t* lastY) : Y0(lastY) {} // used by Row::end()
			Pixel4(
				const uint8_t* Y0, const uint8_t* Y1,
				const uint8_t* U, const uint8_t* V,
				uint32_t* RGB0, uint32_t* RGB1
				)
				: Y0(Y0)
				, Y1(Y1)
				, U(U)
				, V(V)
				, RGB0(RGB0)
				, RGB1(RGB1)
			{}

			Pixel4(const Pixel4&) = default;
			Pixel4& operator=(const Pixel4&) = default;

			bool operator != (const Pixel4& rhs) const { return Y0 != rhs.Y0; }
			Pixel4& operator++()
			{
				++RGB0; ++RGB0;
				++RGB1; ++RGB1;

				++Y0; ++Y0;
				++Y1; ++Y1;
				++U;
				++V;

				return *this;
			}

			Pixel4& operator*() { return *this; }

			void moveAsGrayscale()
			{
				RGB0[0] = RGB24(Y0[0], Y0[0], Y0[0]);
				RGB0[1] = RGB24(Y0[1], Y0[1], Y0[1]);
				RGB1[0] = RGB24(Y1[0], Y1[0], Y1[0]);
				RGB1[1] = RGB24(Y1[1], Y1[1], Y1[1]);
			}

			void moveAsColor()
			{
				int u = *U - 128;
				int v = *V - 128;

				int y00 = Y0[0] - 16;
				int y01 = Y0[1] - 16;
				int y10 = Y1[0] - 16;
				int y11 = Y1[1] - 16;

				RGB0[0] = YUV(y00, u, v);
				RGB0[1] = YUV(y01, u, v);
				RGB1[0] = YUV(y10, u, v);
				RGB1[1] = YUV(y11, u, v);
			}

		private:
			static inline uint32_t YUV(int y, int u, int v)
			{
				y *= 298;
				return RGB24(
					/* R */ clamp((y + 409 * v + 128) >> 8),
					/* G */ clamp((y - 100 * u - 208 * v + 128) >> 8),
					/* B */ clamp((y + 516 * u + 128) >> 8)
					);
			}
		};

		struct Row
		{
			Pixel4 row;
			size_t width;
			Row(const Pixel4& row, size_t halfWidth) : row(row), width(halfWidth << 1) {}

			Pixel4 begin() { return row; }
			Pixel4 end() { return Pixel4(row.Y0 + width); }
		};

		struct RowIterator
		{
			PlanarYUV420* parent;
			size_t row;
			RowIterator(PlanarYUV420* parent, size_t row) : parent(parent), row(row) {}
			RowIterator(const RowIterator&) = default;
			RowIterator& operator=(const RowIterator&) = default;

			bool operator != (const RowIterator& rhs) const { return row != rhs.row; }
			RowIterator& operator++() { ++row; return *this; }
			//RowIterator operator++(int) { auto tmp = *this;  row++; return tmp; }

			Row operator*()
			{
				return{ parent->row4(row), parent->halfWidth };
			}
		};

		PlanarYUV420(int w, int h, const void* in, uint32_t* out)
			: halfWidth(w >> 1)
			, halfHeight(h >> 1)
			, Yplane(w * h)
			, UVplane((w >> 1) * (h >> 1))
			, in((const uint8_t*)in)
			, out(out)
		{}

		RowIterator begin() { return RowIterator(this, 0); }
		RowIterator end() { return RowIterator(this, halfHeight); }

		// returns Pixel4 - a 4-pixel YUV420 cluster with output attached
		Pixel4 row4(size_t half_y)
		{
			auto Y = in;
			auto U = Y + Yplane;
			auto V = U + UVplane;
			auto Ystride = halfWidth << 1;
			auto row = half_y << 1;
			return{
				Y + (row * Ystride),
				Y + ((row + 1) * Ystride),
				U + half_y * halfWidth,
				V + half_y * halfWidth,
				out + (row * Ystride),
				out + ((row + 1) * Ystride)
			};
		}
	};

	struct buffer
	{
		uint32_t * data;
		int width, height;
	};
}

#endif // __GFX_BASIC_HPP__
