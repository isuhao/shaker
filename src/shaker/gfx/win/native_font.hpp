#ifndef __GFX_WIN_NATIVE_FONT_HPP__
#define __GFX_WIN_NATIVE_FONT_HPP__

#include <shaker/gfx/font.hpp>
#include <memory>
#include <vector>
#include <mutex>
#include <Windows.h>

namespace gfx
{
	namespace font
	{
		namespace win
		{
			static const size_t CACHE_SIZE = 4;

			class GdiGlyph
			{
				uint32_t m_code_point;
				long m_advance;
				bool m_loaded;
				std::vector<uint8_t> m_pixmap;
				int m_width;
				int m_height;
				int m_offset_x;
				int m_offset_y;
			public:
				GdiGlyph(HDC dc, uint32_t code_point);
				uint32_t code_point() const { return m_code_point; }
				long advance() const { return m_advance; }
				bool loaded() const { return m_loaded; }
				const uint8_t* pixmap() { return m_pixmap.data(); }
				int width() const { return m_width; }
				int height() const { return m_height; }
				int offset_x() const { return m_offset_x; }
				int offset_y() const { return m_offset_y; }
			};

			typedef std::shared_ptr<GdiGlyph> glyph_ptr;

			class GdiFont
			{
				std::mutex m;
				HFONT m_hFont;
				HDC m_hDC;
				TEXTMETRIC m_metrics;
				std::string m_family_name;
				int m_size;
				bool m_bold;
				bool m_italic;
				long m_space_adv;
				std::vector<glyph_ptr> cache;
			public:
				GdiFont(HFONT font, const std::string& family_name, int size, bool bold, bool italic);
				~GdiFont();
				const std::string& family_name() const { return m_family_name; }
				int size() const { return m_size; }
				bool isBold() const { return m_bold; }
				bool isItalic() const { return m_italic; }
				long height() const { return m_metrics.tmHeight; }
				long asc() const { return m_metrics.tmAscent; }
				long desc() const { return m_metrics.tmDescent; }
				long interline() const { return m_metrics.tmExternalLeading; }
				long space();
				long adv(uint32_t code_point);
				glyph_ptr glyph(uint32_t code_point);
			};

			typedef std::shared_ptr<GdiFont> gdi_ptr;

			class Repo
			{
				std::mutex m;
				std::vector<gdi_ptr> cache;
				static Repo& repo()
				{
					static Repo self;
					return self;
				}

				gdi_ptr _load(const std::string& family_name, int size, bool bold, bool italic);

			public:
				static gdi_ptr load(const std::string& family_name, int size, bool bold, bool italic)
				{
					return repo()._load(family_name, size, bold, italic);
				}
			};

			class Font : public font::Font
			{
				gdi_ptr rep;
			public:
				Font(const gdi_ptr& rep) : rep(rep) {}
				long height() const override { return rep->height(); }
				long asc() const override { return rep->asc(); }
				long desc() const override { return rep->desc(); }
				long line_height() const override { return rep->height() + rep->interline(); }
				void paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const override;
				std::tuple<size_t, size_t> textSize(const std::string& utf8) const override;
			};
		}
	}
}

#endif //__GFX_WIN_NATIVE_FONT_HPP__
