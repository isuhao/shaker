#include <shaker/gfx/canvas.hpp>
#include <shaker/gfx/basic.hpp>
#include <shaker/gfx/bitmap.hpp>
#include <shaker/gfx/alpha_bitmap.hpp>
#include <shaker/gfx/palette_bitmap.hpp>
#include <utility>

namespace gfx
{
	Canvas::Canvas(uint32_t* data, int width, int height, int stride)
		: m_data(data)
		, m_width(width)
		, m_height(height)
		, m_stride(stride ? stride : width)
	{
	}

	bool Canvas::update_pos(int& x, int& y, int& w, int& h, int& ox, int& oy) const
	{
		ox = oy = 0;

		if (x < 0)
		{
			x = -x;
			if (x > w)
				return false;

			ox = x;
			w -= x;
			x = 0;
		}

		if (y < 0)
		{
			y = -y;
			if (y > h)
				return false;

			oy = y;
			h -= y;
			y = 0;
		}

		if (x >= m_width || y >= m_height)
			return false;

		if (m_width - x < w)
			w = m_width - x;

		if (m_height - y < h)
			h = m_height - y;

		if (!w || !h)
			return false;

		return true;
	}

	void Canvas::rect(uint32_t color, int x, int y, int w, int h)
	{
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();

		uint8_t
			a = (color >> 24) & 0xFF,
			r = (color >> 16) & 0xFF,
			g = (color >> 8) & 0xFF,
			b = (color)& 0xFF;

		if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
			std::swap(r, b);

		int ignore;
		if (!update_pos(x, y, w, h, ignore, ignore))
			return;

		uint32_t* dst = m_data + x + y * m_stride;

		if (a == 0)
		{
			return;
		}

		if (a == 255)
		{
			for (y = 0; y < h; ++y)
			{
				auto dst_row = dst + y * m_stride;

				for (x = 0; x < w; ++x)
					*dst_row++ = color;
			}
			return;
		}

		for (y = 0; y < h; ++y)
		{
			auto dst_row = dst + y * m_stride;

			for (x = 0; x < w; ++x)
			{
				auto under = *dst_row;
				uint8_t
					R = (under >> 16) & 0xFF,
					G = (under >> 8) & 0xFF,
					B = (under)& 0xFF;

				if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
					std::swap(R, B);

				*dst_row++ =
					gfx::RGB24(
					gfx::blend(a, r, R),
					gfx::blend(a, g, G),
					gfx::blend(a, b, B)
					);
			}
		}
	}

	void Canvas::put_pixel(int x, int y, uint32_t color)
	{
		if (x < 0 || y < 0 || x >= m_width || y >= m_height)
			return;

		auto dest = m_data + x + y * m_stride;
		*dest = color;
	}

	template <typename BitmapT, typename SourceLine, typename Copy>
	void Canvas::paint(int x, int y, int w, int h, int ox, int oy, const BitmapT& bmp, SourceLine sourceLine, Copy copy)
	{
		auto dest = m_data + x + y * m_stride;
		auto source = bmp.m_data + ox + oy * bmp.m_stride;

		for (int y = 0; y < h; ++y)
		{
			uint32_t* dst = dest + y * m_stride;
			auto src = sourceLine(source, y, bmp.m_stride, w);

			for (int x = 0; x < w; ++x)
				copy(src, dst);
		}
	}

	void Canvas::paint(int x, int y, const Bitmap& bmp)
	{
		int w = bmp.width();
		int h = bmp.height();
		int offset_x, offset_y;

		bool mirrored = false;
		if (w < 0)
		{
			mirrored = true;
			w = -w;
		}

		if (!update_pos(x, y, w, h, offset_x, offset_y))
			return;

		if (mirrored)
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint32_t* source, int y, int stride, int width){ return source + y * stride + width - 1; },
				[](const uint32_t*& src, uint32_t*& dst){ *dst++ = *src--; });
		}
		else
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint32_t* source, int y, int stride, int){ return source + y * stride; },
				[](const uint32_t*& src, uint32_t*& dst){ *dst++ = *src++; });
		}
	}

	void Canvas::paint(int x, int y, const AlphaBitmap& bmp)
	{
		int w = bmp.width();
		int h = bmp.height();
		int offset_x, offset_y;

		bool mirrored = false;
		if (w < 0)
		{
			mirrored = true;
			w = -w;
		}

		if (!update_pos(x, y, w, h, offset_x, offset_y))
			return;

		auto dest = m_data + x + y * m_stride;
		auto source = bmp.m_data + offset_x + offset_y * bmp.m_stride;

		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
		auto blend = [format](const uint32_t*& src, uint32_t*& dst) -> const uint32_t*&
		{
			uint32_t color = *src;
			uint8_t
				a = (color >> 24) & 0xFF,
				r = (color >> 16) & 0xFF,
				g = (color >> 8 ) & 0xFF,
				b = (color      ) & 0xFF;

			if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
				std::swap(r, b);

			if (!a)
			{
				dst++;
				return src;
			}

			if (a == 255)
			{
				*dst++ = *src;
				return src;
			}

			auto under = *dst;
			uint8_t
				R = (under >> 16) & 0xFF,
				G = (under >> 8) & 0xFF,
				B = (under)& 0xFF;

			if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
				std::swap(R, B);

			*dst++ =
				gfx::RGB24(
				gfx::blend(a, r, R),
				gfx::blend(a, g, G),
				gfx::blend(a, b, B)
				);

			return src;
		};

		if (mirrored)
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint32_t* source, int y, int stride, int width){ return source + y * stride + width - 1; },
				[blend](const uint32_t*& src, uint32_t*& dst){ blend(src, dst)--; });
		}
		else
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint32_t* source, int y, int stride, int){ return source + y * stride; },
				[blend](const uint32_t*& src, uint32_t*& dst){ blend(src, dst)++; });
		}
	}

	void Canvas::paint(int x, int y, const PaletteBitmap& bmp)
	{
		int w = bmp.width();
		int h = bmp.height();
		int offset_x, offset_y;

		bool mirrored = false;
		if (w < 0)
		{
			mirrored = true;
			w = -w;
		}

		if (!update_pos(x, y, w, h, offset_x, offset_y))
			return;

		auto dest = m_data + x + y * m_stride;
		auto source = bmp.m_data + offset_x + offset_y * bmp.m_stride;
		auto palette = bmp.m_palette;

		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
		auto blend = [format, palette](const uint8_t*& src, uint32_t*& dst) -> const uint8_t*&
		{
			uint32_t color = palette[*src];
			uint8_t
				a = (color >> 24) & 0xFF,
				r = (color >> 16) & 0xFF,
				g = (color >> 8 ) & 0xFF,
				b = (color      ) & 0xFF;

			if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
				std::swap(r, b);

			if (!a)
			{
				dst++;
				return src;
			}

			if (a == 255)
			{
				*dst++ = color;
				return src;
			}

			auto under = *dst;
			uint8_t
				R = (under >> 16) & 0xFF,
				G = (under >> 8) & 0xFF,
				B = (under)& 0xFF;

			if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
				std::swap(R, B);

			*dst++ =
				gfx::RGB24(
				gfx::blend(a, r, R),
				gfx::blend(a, g, G),
				gfx::blend(a, b, B)
				);

			return src;
		};

		if (mirrored)
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint8_t* source, int y, int stride, int width){ return source + y * stride + width - 1; },
				[blend](const uint8_t*& src, uint32_t*& dst){ blend(src, dst)--; });
		}
		else
		{
			paint(x, y, w, h, offset_x, offset_y, bmp,
				[](const uint8_t* source, int y, int stride, int){ return source + y * stride; },
				[blend](const uint8_t*& src, uint32_t*& dst){ blend(src, dst)++; });
		}
	}
}