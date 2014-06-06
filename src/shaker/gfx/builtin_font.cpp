#include "builtin_font.hpp"
#include <shaker/gfx/palette_bitmap.hpp>
#include <shaker/gfx/utf8.hpp>

namespace gfx { namespace font
{
	namespace
	{
#include "glyphs.inl"

		static const int glyphs = sizeof(alphabet)/sizeof(alphabet[0])-1;
		static const int glyph_width = 7;
		static const int glyph_stride = glyph_width;
		static const int glyph_height = sizeof(pixmap) / (glyph_stride * glyphs);
		static const int glyph_desc = 3;
		static const int glyph_asc = glyph_height - glyph_desc;
		static const int interline = 2;

		int glyph_id(wchar_t c)
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
			const uint8_t* src = pixmap + glyph * glyph_width * glyph_height;
			canvas->paint(x, y, gfx::PaletteBitmap{ (uint8_t*)src, palette, glyph_width, glyph_height });
		}
	}

	ptr builtin()
	{
		return std::make_shared<BuiltIn>();
	}

	long BuiltIn::height() const
	{
		return glyph_height;
	}

	long BuiltIn::asc() const
	{
		return glyph_asc;
	}

	long BuiltIn::desc() const
	{
		return glyph_desc;
	}

	long BuiltIn::line_height() const
	{
		return glyph_height + interline;
	}

	void BuiltIn::paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const
	{
		uint32_t palette[256];
		color &= 0x00FFFFFF;
		for (uint32_t alpha = 0; alpha < 0x100; ++alpha)
			palette[alpha] = color | (alpha << 24);

		auto cr = x;

		auto text = utf8::to32(utf8);
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

	std::tuple<size_t, size_t> BuiltIn::textSize(const std::string& utf8) const
	{
		size_t width = 0;
		size_t height = 1;
		size_t line = 0;

		auto text = utf8::to32(utf8);
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
