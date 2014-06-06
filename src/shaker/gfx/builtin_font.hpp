#ifndef __GFX_BUILTIN_FONT_HPP__
#define __GFX_BUILTIN_FONT_HPP__

#include <shaker/gfx/font.hpp>

namespace gfx
{
	namespace font
	{
		class BuiltIn : public Font
		{
		public:
			long height() const override;
			long asc() const override;
			long desc() const override;
			long line_height() const override;
			void paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const override;
			std::tuple<size_t, size_t> textSize(const std::string& utf8) const override;
		};
	}
}
#endif // __GFX_BUILTIN_FONT_HPP__
