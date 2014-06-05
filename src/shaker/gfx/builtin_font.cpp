#include "builtin_font.hpp"
#include <shaker/gfx/palette_bitmap.hpp>

namespace gfx { namespace font
{
	namespace
	{
#include "glyphs.inl"

		static const int glyphs = sizeof(alphabet)-1;
		static const int glyph_width = 7;
		static const int glyph_stride = glyph_width;
		static const int glyph_height = sizeof(pixmap) / (glyph_stride * glyphs);
		static const int interline = 2;

		int glyph_id(char c)
		{
			for (int i = 0; i < glyphs; ++i)
			{
				if (alphabet[i] == c)
					return i;
			}
			return -1;
		}

		void paint(int glyph, int x, int y, const uint32_t* palette, Canvas* canvas)
		{
#if 0
			int g_w = glyph_width;
			int g_h = glyph_height;

			const uint8_t* src = pixmap + glyph * glyph_stride * glyph_height;

			if (x < 0)
			{
				x = -x;
				if (x > g_w)
					return;

				src += x;
				g_w -= x;
				x = 0;
			}

			if (y < 0)
			{
				y = -y;
				if (y > g_h)
					return;

				src += y * glyph_stride;
				g_h -= y;
				y = 0;
			}

			if (x >= canvas->width() || y >= canvas->height())
				return;

			if (canvas->width() - x < g_w)
				g_w = canvas->width() - x;

			if (canvas->height() - y < g_h)
				g_h = y - canvas->height();

			if (!g_w || !g_h)
				return;

			uint32_t* dst = canvas->data() + x + y * canvas->stride();
			PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();

			for (y = 0; y < g_h; ++y)
			{
				auto src_row = src + y * glyph_stride;
				auto dst_row = dst + y * canvas->stride();

				for (x = 0; x < g_w; ++x)
				{
					auto r = *src_row++;
					auto g = *src_row++;
					auto b = *src_row++;
					auto a = *src_row++;
					if (!a)
					{
						dst_row++;
						continue;
					}

					auto under = *dst_row;
					uint8_t
						R = (under >> 16) & 0xFF,
						G = (under >> 8) & 0xFF,
						B = (under)& 0xFF;

					if (format != PP_IMAGEDATAFORMAT_BGRA_PREMUL)
						std::swap(R, B);

					*dst_row++ =
						RGB24(
						blend(a, r, R),
						blend(a, g, G),
						blend(a, b, B)
						);
				}
			}
#else
			//const uint8_t* src = pixmap + glyph * glyph_stride * glyph_height;
			const uint8_t* src = pixmap + glyph * glyph_width * glyph_height;
			canvas->paint(x, y, gfx::PaletteBitmap{ (uint8_t*)src, palette, glyph_width, glyph_height });
#endif
		}
	}

	ptr builtin()
	{
		return std::make_shared<BuiltIn>();
	}

	size_t BuiltIn::height() const
	{
		return glyph_height;
	}

	size_t BuiltIn::line_height() const
	{
		return glyph_height + interline;
	}

	void BuiltIn::paint(const std::string& text, int x, int y, uint32_t color, Canvas* canvas) const
	{
		uint32_t palette[256];
		color &= 0x00FFFFFF;
		for (uint32_t alpha = 0; alpha < 0x100; ++alpha)
			palette[alpha] = color | (alpha << 24);

		auto cr = x;

		for (auto&& c : text)
		{
			if (c == ' ')
			{
				x += glyph_width;
				continue;
			}
			if (c == '\n')
			{
				y += line_height();
				x = cr;
				continue;
			}
			auto glyph = glyph_id(c);
			if (glyph < 0) continue;
			font::paint(glyph, x, y, palette, canvas);
			x += glyph_width;
		}
	}

	std::tuple<size_t, size_t> BuiltIn::textSize(const std::string& text) const
	{
		size_t width = 0;
		size_t height = 1;
		size_t line = 0;

		for (auto&& c : text)
		{
			if (c == '\n')
			{
				height++;
				if (width < line)
					width = line;
				line = 0;
				continue;
			}
			if (c == ' ')
			{
				line++;
				continue;
			}
			auto glyph = glyph_id(c);
			if (glyph < 0) continue;
			line++;
		}
		if (width < line)
			width = line;

		return std::make_tuple(
			width * glyph_width,
			height * glyph_height + (height - 1) * interline
			);
	}
}} // gfx::font
