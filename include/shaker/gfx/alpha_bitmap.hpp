#ifndef __GFX_ALPHA_BITMAP_HPP__
#define __GFX_ALPHA_BITMAP_HPP__

#include <stdint.h>

namespace gfx
{
	class Canvas;
	class AlphaBitmap
	{
		friend class Canvas;
		uint32_t* m_data;
		int m_width, m_height, m_stride;
	public:
		AlphaBitmap(uint32_t* data, int width, int height, int stride = 0)
			: m_data(data)
			, m_width(width)
			, m_height(height)
			, m_stride(stride ? stride : width)
		{
		}

		int width() const { return m_width; }
		int height() const { return m_height; }
	};
}

#endif // __GFX_ALPHA_BITMAP_HPP__
