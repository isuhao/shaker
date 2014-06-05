#ifndef __GFX_PALETTE_BITMAP_HPP__
#define __GFX_PALETTE_BITMAP_HPP__

#include <stdint.h>

namespace gfx
{
	class Canvas;
	class PaletteBitmap
	{
		friend class Canvas;
		uint8_t* m_data;
		const uint32_t* m_palette;
		int m_width, m_height, m_stride;
	public:
		PaletteBitmap(uint8_t* data, const uint32_t* palette, int width, int height, int stride = 0)
			: m_data(data)
			, m_palette(palette)
			, m_width(width)
			, m_height(height)
			, m_stride(stride ? stride : width)
		{
		}

		int width() const { return m_width; }
		int height() const { return m_height; }
	};
}

#endif // __GFX_PALETTE_BITMAP_HPP__
