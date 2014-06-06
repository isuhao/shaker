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

			virtual long height() const = 0;
			virtual long asc() const = 0;
			virtual long desc() const = 0;
			virtual long line_height() const = 0;
			virtual void paint(const std::string& utf8, int x, int y, uint32_t color, Canvas* canvas) const = 0;
			virtual std::tuple<size_t, size_t> textSize(const std::string& utf8) const = 0;
		};

		typedef std::shared_ptr<Font> ptr;

		ptr builtin();
		ptr load(const std::string& family_name, int size, bool bold, bool italic);
	}
}

#endif // __GFX_FONT_HPP__
