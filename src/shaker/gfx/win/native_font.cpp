#include "native_font.hpp"
#include <shaker/gfx/palette_bitmap.hpp>
#include <shaker/gfx/utf8.hpp>

namespace std
{
#define MK_DELETER(TYPE, FN) \
	template<> \
	struct default_delete<TYPE ## __> \
	{ \
		typedef default_delete<TYPE ## __> _Myt; \
		default_delete() {} \
		template<class _Ty2, class = typename enable_if<is_convertible<_Ty2 *, TYPE>::value, void>::type> \
		default_delete(const default_delete<_Ty2>&) {} \
		void operator()(TYPE _Ptr) const \
		{ \
			if (_Ptr) \
				FN(_Ptr); \
		} \
	}

	MK_DELETER(HBITMAP, DeleteObject);
	MK_DELETER(HPEN, DeleteObject);
}

namespace gfx { namespace font {
	namespace win {

		static const size_t interline = 2;

		gdi_ptr gdi_font(const std::string& family_name, int size, bool bold, bool italic);

		// Note that, for the GGO_GRAYn_BITMAP values, the function retrieves a glyph bitmap
		// that contains n^2+1 (n squared plus one) levels of gray.
		uint8_t gray8(uint8_t src)
		{
			static const uint8_t max = 64;
			static const uint8_t out_max = 0xFF;
			return (uint32_t)src * out_max / max;
		}

		GdiGlyph::GdiGlyph(HDC dc, uint32_t code_point)
			: m_code_point(code_point)
			, m_advance(0)
			, m_loaded(false)
			, m_width(0)
			, m_height(0)
		{
			MAT2 mat2 = {};
			mat2.eM11.value = mat2.eM22.value = 1;
			GLYPHMETRICS gm = {};
			int size = GetGlyphOutline(dc, m_code_point, GGO_GRAY8_BITMAP, &gm, 0, nullptr, &mat2);
			if (size == GDI_ERROR)
				return;

			m_advance = gm.gmCellIncX;
			m_width = gm.gmBlackBoxX;
			m_height = gm.gmBlackBoxY;
			m_offset_x = -gm.gmptGlyphOrigin.x;
			m_offset_y = gm.gmptGlyphOrigin.y;

			size_t s_stride = size / m_height;

			std::unique_ptr<uint8_t[]> bitmap{ new uint8_t[size] };
			if (!bitmap)
				return;

			if (GetGlyphOutline(dc, m_code_point, GGO_GRAY8_BITMAP, &gm, size, bitmap.get(), &mat2) == GDI_ERROR)
				return;

			size_t length = m_width * m_height;
			m_pixmap.resize(length);
			if (m_pixmap.size() != length)
				return;

			uint8_t* dest = &m_pixmap[0];

			//wchar_t buffer[1024];
			//swprintf_s(buffer, L"\n%c/%d/U+%x:\noff:<%dx%d> box:<%dx%d> adv:%d\n\n", (wchar_t)m_code_point, m_code_point, m_code_point, -m_offset_x, m_offset_y, m_width, m_height, m_advance);
			//OutputDebugString(buffer);

			for (int y = 0; y < m_height; ++y)
			{
				auto src = bitmap.get() + y * s_stride;
				auto dst = dest + y * m_width;
				//std::string line = "|";
				for (int x = 0; x < m_width; ++x)
				{
					//switch (*src / 8)
					//{
					//case 0: line += "  "; break;
					//case 1: line += ".."; break;
					//case 2: line += "++"; break;
					//case 3: line += "**"; break;
					//case 4: line += "BB"; break;
					//case 5: line += "&&"; break;
					//case 6: line += "@@"; break;
					//case 7: line += "%%"; break;
					//case 8: line += "##"; break;
					//}
					*dst++ = gray8(*src++);
				}
				//line += "|\n";
				//OutputDebugStringA(line.c_str());
			}

			m_loaded = true;
		}

		GdiFont::GdiFont(HFONT font, const std::string& family_name, int size, bool bold, bool italic)
			: m_hFont(font)
			, m_hDC(CreateCompatibleDC(nullptr))
			, m_family_name(family_name)
			, m_size(size)
			, m_bold(bold)
			, m_italic(italic)
			, m_space_adv(-1)
		{
			SelectObject(m_hDC, m_hFont);
			GetTextMetrics(m_hDC, &m_metrics);
		}

		GdiFont::~GdiFont()
		{
			if (m_hDC)
				DeleteDC(m_hDC);
			if (m_hFont)
				DeleteObject(m_hFont);
		}

		long GdiFont::space()
		{
			if (m_space_adv < 0)
			{
				std::lock_guard<std::mutex> lock(m);
				if (m_space_adv < 0)
				{
					ABC abc;
					GetCharABCWidths(m_hDC, ' ', ' ', &abc);
					m_space_adv = abc.abcA + abc.abcB + abc.abcC;
				}
			}

			return m_space_adv;
		}

		long GdiFont::adv(uint32_t code_point)
		{
			auto g = glyph(code_point);
			return g ? g->advance() : 0;
		}

		glyph_ptr GdiFont::glyph(uint32_t code_point)
		{
			std::lock_guard<std::mutex> lock(m);

			glyph_ptr ret;
			for (auto&& glyph : cache)
			{
				if (glyph->code_point() == code_point)
					return glyph;
			}

			ret = std::make_shared<GdiGlyph>(m_hDC, code_point);
			cache.push_back(ret);
			return ret;
		}

		gdi_ptr Repo::_load(const std::string& family_name, int size, bool bold, bool italic)
		{
			std::lock_guard<std::mutex> guard(m);

			auto cur = cache.begin();
			auto end = cache.end();

			for (; cur != end; ++cur)
			{
				auto&& ptr = *cur;
				if (ptr->family_name() != family_name ||
					ptr->size() != size ||
					ptr->isBold() != bold ||
					ptr->isItalic() != italic)
				{
					continue;
				}

				cache.erase(cur);
				cache.insert(cache.begin(), ptr);

				return ptr;
			}

			auto ret = gdi_font(family_name, size, bold, italic);

			if (cache.size() >= CACHE_SIZE)
				cache.pop_back();
			cache.insert(cache.begin(), ret);

			return ret;
		}

		void Font::paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const
		{
			uint32_t palette[256];
			color &= 0x00FFFFFF;
			for (uint32_t alpha = 0; alpha < 0x100; ++alpha)
				palette[alpha] = color | (alpha << 24);

			auto cr = x;
			auto ascend = rep->asc();

			y += rep->asc();

			auto text = utf8::to32(utf8);
			for (auto&& c : text)
			{
				if (c == ' ')
				{
					x += rep->space();
					continue;
				}
				if (c == '\n')
				{
					y += line_height();
					x = cr;
					continue;
				}

				auto glyph = rep->glyph(c);
				if (!glyph)
					continue;

				if (glyph->loaded())
				{
					canvas->paint(
						x - glyph->offset_x(), y - glyph->offset_y(),
						gfx::PaletteBitmap{ (uint8_t*)glyph->pixmap(), palette, glyph->width(), glyph->height() }
					);
				}

				x += glyph->advance();
			}
		}

		std::tuple<size_t, size_t> Font::textSize(const std::string& utf8) const
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
					line += rep->space();
					continue;
				}

				line += rep->adv(c);
			}
			if (width < line)
				width = line;

			return std::make_tuple( width, height * rep->height() + (height - 1) * rep->interline());
		}

		gdi_ptr gdi_font(const std::string& family_name, int size, bool bold, bool italic)
		{
			HFONT font = CreateFontA(size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, italic ? TRUE : FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, family_name.c_str());
			return std::make_shared<GdiFont>(font, family_name, size, bold, italic);
		}
	}

	ptr load(const std::string& family_name, int size, bool bold, bool italic)
	{
		return std::make_shared<win::Font>( win::Repo::load(family_name, size, bold, italic) );
	}
}}
