#ifndef __GFX_FONT_HPP__
#define __GFX_FONT_HPP__

#include <memory>
#include <string>
#include <tuple>

#include <shaker/gfx/basic.hpp>
#include <shaker/gfx/canvas.hpp>

namespace gfx
{
	namespace font
	{
		struct Font
		{
			virtual ~Font() {}

			virtual size_t height() const = 0;
			virtual size_t asc() const = 0;
			virtual size_t desc() const = 0;
			virtual size_t line_height() const = 0;
			virtual void paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const = 0;
			virtual std::tuple<size_t, size_t> textSize(const std::string& utf8) const = 0;
		};

		typedef std::shared_ptr<Font> ptr;

		ptr builtin();
	}
}

#endif // __GFX_FONT_HPP__
