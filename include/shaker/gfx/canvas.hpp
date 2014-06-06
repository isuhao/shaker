#ifndef __GFX_CANVAS_HPP__
#define __GFX_CANVAS_HPP__

#include <stdint.h>

namespace gfx
{
	class Bitmap;
	class AlphaBitmap;
	class PaletteBitmap;

	class Canvas
	{
		uint32_t* m_data;
		int m_width, m_height, m_stride;

		inline bool update_pos(int& x, int& y, int& w, int& h, int& ox, int& oy) const;

		template <typename BitmapT, typename SourceLine, typename Copy>
		void paint(int x, int y, int w, int h, int ox, int oy, const BitmapT& bmp, SourceLine sourceLine, Copy copy);
	public:
		Canvas(uint32_t* data, int width, int height, int stride = 0);

		//uint32_t* data() { return m_data; }
		int width() const { return m_width; }
		int height() const { return m_height; }
		//int stride() const { return m_stride; }

		void rect(uint32_t color, int x, int y, int w, int h);
		void put_pixel(int x, int y, uint32_t color);
		void paint(int x, int y, const Bitmap& bmp);
		void paint(int x, int y, const AlphaBitmap& bmp);
		void paint(int x, int y, const PaletteBitmap& bmp);
	};
}

#endif // __GFX_CANVAS_HPP__
