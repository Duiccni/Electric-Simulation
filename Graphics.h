#pragma once

#include "Point.h"
#include <Windows.h>

#include <iostream>
#include <numeric>

#define DEFAULT_SURFACE_SIZE 10
constexpr int HALF_DEFAULT_SURFACE_SIZE = DEFAULT_SURFACE_SIZE / 2;
constexpr size_t SQUARE_DEFAULT_SURFACE_SIZE = DEFAULT_SURFACE_SIZE * DEFAULT_SURFACE_SIZE;
constexpr int MINUS_DEFAULT_SURFACE_SIZE = DEFAULT_SURFACE_SIZE - 1;

typedef unsigned int uint;

#define modulo(x, a) ((x) < 0 ? (x) % (a) + (a) : (x) % (a))

#define rgbColor(r, g, b, a) (uint)(((a) << 24) | ((r) << 16) | ((g) << 8) | b)

#define slide(x1, x2, t) (x1 + (x2 - x1) * t)

#define getSign(x) ((x) < 0 ? -1 : 1)

namespace Colors
{
	constexpr uint VISIBLE_BLACK = 0xff000000;
	constexpr uint BLACK = 0;
	constexpr uint GRAY = 0x808080;
	constexpr uint WHITE = 0xffffff;
	constexpr uint RED = 0xff0000;
	constexpr uint GREEN = 0xff00;
	constexpr uint BLUE = 0xff;
	constexpr uint DARK_BLUE = 0x80;
	constexpr uint BETWEEN_GREEN = 0xa000;
	constexpr uint DARK_GREEN = 0x8000;
	constexpr uint DARK_RED = 0x800000;
};

/*
namespace Colors
{
	constexpr uint VISIBLE_BLACK = 0xff000000;
	constexpr uint BLACK = 0;
	constexpr uint GRAY = 0x808080;
	constexpr uint WHITE = 0xffffff;
	constexpr uint RED = 0x40;
	constexpr uint GREEN = 0x80;
	constexpr uint BLUE = 0xff;
	constexpr uint DARK_BLUE = 0xa0;
	constexpr uint DARK_GREEN = 0x30;
	constexpr uint DARK_RED = 0x20;
};
*/

/*
	Kendime Not:

	++i %= 4;
	++i &= 0b11;

	ikiside ayni sey ama 2.si daha hizli.
*/

namespace Graphics
{
	constexpr uint8_t CLAMP_X = 0b1;
	constexpr uint8_t CLAMP_Y = 0b10;
	constexpr uint8_t CLAMP_BOTH = 0b100;

	class Surface
	{
	public:
		uint* buffer = nullptr, * end = nullptr;
		Point size;
		size_t iSize, iBufferSize;

		Point center, minus;

		Surface();
		Surface(Point size, bool createBuffer);
		~Surface();
	};

	Surface::Surface()
	{
		this->size = { DEFAULT_SURFACE_SIZE, DEFAULT_SURFACE_SIZE };
		iSize = SQUARE_DEFAULT_SURFACE_SIZE;
		buffer = new uint[iSize];
		end = buffer + iSize;
		iBufferSize = iSize * sizeof(uint);
		center = { HALF_DEFAULT_SURFACE_SIZE, HALF_DEFAULT_SURFACE_SIZE };
		minus = { MINUS_DEFAULT_SURFACE_SIZE, MINUS_DEFAULT_SURFACE_SIZE };
	}

	Surface::Surface(Point size, bool createBuffer = true)
	{
		this->size = size;
		iSize = static_cast<size_t>(size.x) * size.y;
		if (createBuffer)
		{
			buffer = new uint[iSize];
			end = buffer + iSize;
		}
		iBufferSize = iSize * sizeof(uint);
		center = size / 2;
		minus = size - 1;
	}

	Surface::~Surface()
	{
		delete[] buffer;
	}

	Surface& operator>>(const Surface& src, Surface& dest)
	{
		uint* srcPixel = src.buffer;

		for (uint* destPixel = dest.buffer; destPixel < dest.end; destPixel++, srcPixel++)
			*destPixel = *srcPixel;

		return dest;
	}
};

namespace World
{
	Graphics::Surface sScreen({ 1280, 1080 }, false);
	constexpr int DEFAULT_CELL_SIZE = 32;
	int iCellSize = DEFAULT_CELL_SIZE, idCellSize = iCellSize << 1, ihCellSize = iCellSize >> 1, iqCellSize = iCellSize >> 2, iqhCellSize = iCellSize >> 3, iqCellLeak = 0;
	Point pCellSize = { iCellSize, iCellSize };

	Point pCellOffset = pZero;
	Point pCellOffsetMod = pZero;

	Point pFit = sScreen.size / iCellSize;

	inline bool init()
	{
		sScreen.buffer = (uint*)VirtualAlloc(0, sScreen.iBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		
		if (sScreen.buffer == nullptr)
			return true;
		
		sScreen.end = sScreen.buffer + sScreen.iSize;

		return false;
	}
};

namespace Events
{
	uint64_t error = 0;

	enum ObjectIds
	{
		oHand,
		oShow,
		oWire,
		oBattery,
		oResistor,
	};
};

namespace Graphics
{
	inline void clampToSurface(Point& point, Surface& surface = World::sScreen)
	{
		point.x = clamp(point.x, 0, surface.minus.x);
		point.y = clamp(point.y, 0, surface.minus.y);
	}
};

struct s_mouse
{
	POINT winPos;
	Point pos;
	Point oldPos;
	Point delta;
	Point worldPos;
	Point oldWorldPos;
	Point worldDelta;
	Point roundScreenPos;
	uint* roundScreenPixel;
	bool worldInScreen;
	bool left, middle, right;
};

namespace Data
{
	int targetFps = 50, targetFrameTime = 1000 / targetFps;
	int iTick = 0, iDeltaTime = targetFrameTime, iPerformance = 0, iTopTime = 0;
	float iAvgPerf = 1.0f;
	bool bRunning = true, bDebug = false, bFontMode = false;
	constexpr bool bConsole = false;

	int iCache[2];

	Point pCache[5];
	Point pReserveCache[1];

	inline void swapPCache()
	{
		swapPoint(pCache[0], pCache[1]);
	}

	inline void clampPCache(Graphics::Surface& surface = World::sScreen)
	{
		Graphics::clampToSurface(pCache[0], surface);
		Graphics::clampToSurface(pCache[1], surface);
	}

	struct s_mouse sMouse = { };
};

#define FLOAT1DIV60 0.016666666666666666f

namespace Graphics
{
	uint HSVtoHEX(int h, int s, int v)
	{
		h = modulo(h, 360);
		
		int c = v * s / 255;
		int x = c * (1.0f - fabsf(fmodf(static_cast<float>(h) * FLOAT1DIV60, 2.0f) - 1.0f));

		uint color = v - c;
		uint8_t* b = (uint8_t*)&color;
		b[1] = *b, b[2] = *b;

		int i = (h / 60 + 1) % 6;

		b[2 - (i >> 1)] += c;
		b[i % 3] += x;

		return color;
	}

	Point* worldToScreen(const Point& world, Point& screen, bool force = false, int automatic = 0)
	{
		force = !(force || automatic);

		screen.x = world.x * World::iCellSize + World::pCellOffset.x;
		if (force && (screen.x < 0 || screen.x >= World::sScreen.size.x))
			return nullptr;

		screen.y = world.y * World::iCellSize + World::pCellOffset.y;
		if (force && (screen.y < 0 || screen.y >= World::sScreen.size.y))
			return nullptr;

		if (automatic)
		{
			if (automatic & CLAMP_X)
				screen.x = clamp(screen.x, 0, World::sScreen.minus.x);
			else if (automatic & CLAMP_Y)
				screen.y = clamp(screen.y, 0, World::sScreen.minus.y);
			else if (automatic & CLAMP_BOTH)
				clampToSurface(screen);
		}

		return &screen;
	}

	constexpr uint8_t TOP_OUTSIDE = 0b1;
	constexpr uint8_t BOTTOM_OUTSIDE = 0b10;
	constexpr uint8_t LEFT_OUTSIDE = 0b100;
	constexpr uint8_t RIGHT_OUTSIDE = 0b1000;

	uint8_t worldToScreenWithData(const Point& world, Point& screen)
	{
		screen = world * World::iCellSize + World::pCellOffset;

		uint8_t data = 0;

		if (screen.y < 0)
			data |= TOP_OUTSIDE;
		if (screen.y >= World::sScreen.size.y)
			data |= BOTTOM_OUTSIDE;
		if (screen.x < 0)
			data |= LEFT_OUTSIDE;
		if (screen.x >= World::sScreen.size.x)
			data |= RIGHT_OUTSIDE;

		return data;
	}

	void screenToWorld(const Point& screen, Point& world)
	{
		world.x = screen.x - World::pCellOffset.x;
		world.x = (world.x < 0 ? world.x - World::ihCellSize : world.x + World::ihCellSize) / World::iCellSize;

		world.y = screen.y - World::pCellOffset.y;
		world.y = (world.y < 0 ? world.y - World::ihCellSize : world.y + World::ihCellSize) / World::iCellSize;
	}

	inline bool isInside(const Point& point, const Surface& surface = World::sScreen)
	{
		return point.x >= 0 && point.y >= 0 && point.x < surface.size.x && point.y < surface.size.y;
	}

	inline uint* getRawPixel(const Point& point, const Surface& surface = World::sScreen)
	{
		return surface.buffer + point.x + point.y * surface.size.x;
	}
	
	inline uint* getRawPixel(const Point& point, Surface* surface)
	{
		return surface->buffer + point.x + point.y * surface->size.x;
	}

	inline uint* getPixel(const Point& point, const Surface& surface = World::sScreen)
	{
		return isInside(point, surface) ? getRawPixel(point, surface) : nullptr;
	}

	inline void setSurePixel(const Point& point, uint color, Surface& surface = World::sScreen)
	{
		*getRawPixel(point, surface) = color;
	}

	inline void setPixel(const Point& point, uint color, Surface& surface = World::sScreen)
	{
		if (uint* pixel = getPixel(point, surface))
			*pixel = color;
	}

	inline void clear(uint color, Surface& surface = World::sScreen)
	{
		for (uint* pixel = surface.buffer; pixel < surface.end; pixel++)
			*pixel = color;
	}

	inline Point getPosition(uint* pixel, const Surface& surface = World::sScreen)
	{
		int index = pixel - surface.buffer;
		return { index % surface.size.x, index / surface.size.x };
	}

	void straightenLine(const Point& start, Point& end)
	{
		if (start.x == end.x || start.y == end.y)
			return;

		if (abs(end.y - start.y) > abs(end.x - start.x)) // Equals slope
			end.x = start.x;
		else
			end.y = start.y;
	}

	// Uses Data::pCache[0, 1]
	void blitSurface(Surface& dest, const Surface& src, Point point, bool alpha = true, bool given = false)
	{
		Point size, bias;

		if (given)
		{
			size = Data::pCache[2];
			bias = Data::pCache[3];
		}
		else
		{
			Data::pCache[0] = point;
			Data::pCache[1] = point + src.size;

			Data::clampPCache(dest);

			size = Data::pCache[1] - Data::pCache[0];

			if (size.x == 0 || size.y == 0)
				return;

			bias = Data::pCache[0] - point;
		}

		uint* srcPixel = getRawPixel(bias, src);
		uint* pixel = getRawPixel(Data::pCache[0], dest);

		int yStep = dest.size.x - size.x, ySrcStep = src.size.x - size.x;

		if (alpha)
		{
			for (uint* end = pixel + size.y * (yStep + size.x); pixel < end; pixel += yStep, srcPixel += ySrcStep)
				for (uint* xEnd = pixel + size.x; pixel < xEnd; pixel++, srcPixel++)
					if (*srcPixel)
						*pixel = *srcPixel;
		}
		else
		{
			for (uint* end = pixel + size.y * (yStep + size.x); pixel < end; pixel += yStep, srcPixel += ySrcStep)
				for (uint* xEnd = pixel + size.x; pixel < xEnd; pixel++, srcPixel++)
					*pixel = *srcPixel;
		}

		/*
		uint* end = pixel + size.y * (yStep + size.x);

		if (alpha)
		{
			while (pixel < end)
			{
				for (uint* xEnd = pixel + size.x; pixel < xEnd; pixel++, srcPixel++)
					if (*srcPixel)
						*pixel = *srcPixel;

				pixel += yStep;
				srcPixel += ySrcStep;
			}
		}
		else
		{
			while (pixel < end)
			{
				for (uint* xEnd = pixel + size.x; pixel < xEnd;)
					*pixel++ = *srcPixel++;
				pixel += yStep;
				srcPixel += ySrcStep;
			}
		}
		*/
	}
	
	Surface* resizeSurface(Surface& surface, Point size)
	{
		Surface* newSurface = new Surface(size, true);

		Point mainStep = surface.size / size;
		mainStep.y *= surface.size.x;

		Point error = pZero;

		uint* pixel = newSurface->buffer;
		uint* srcPixelStart = surface.buffer;
		uint* srcPixel = nullptr;

		Point xErr = { size.x, surface.size.x % size.x };
		Point yErr = { size.y, surface.size.y % size.y };

		while (pixel < newSurface->end)
		{
			error.x = 0;
			srcPixel = srcPixelStart;
			for (uint* xEnd = pixel + size.x; pixel < xEnd; pixel++)
			{
				*pixel = *srcPixel;
				srcPixel += mainStep.x;

				if (error.x >= xErr.x)
				{
					srcPixel++;
					error.x -= xErr.x;
				}
				error.x += xErr.y;
			}

			srcPixelStart += mainStep.y;
			if (error.y >= yErr.x)
			{
				srcPixelStart += surface.size.x;
				error.y -= yErr.x;
			}
			error.y += yErr.y;
		}

		return newSurface;
	}

	void copySurfacePresize(const Surface& src, Surface& dest, Point pos)
	{
		int yStep = src.size.x - dest.size.x;
		if (pos.x < 0 || pos.y < 0 || pos.x > yStep || pos.y + dest.size.y > src.size.y)
			return;

		uint* pixel = dest.buffer, * srcPixel = getRawPixel(pos, src);
		for (int y = 0; y < dest.size.y; y++)
		{
			for (int x = 0; x < dest.size.x; x++)
				*pixel++ = *srcPixel++;
			srcPixel += yStep;
		}
	}

	/*
	void blurSurface(Surface& surface)
	{
		Surface* newSurface = new Surface(surface.size);

		int yStep = surface.size.x * 4;

		uint8_t* srcPixel = (uint8_t*)surface.buffer + 4 + yStep;
		uint8_t* end = (uint8_t*)newSurface->end - 4 - yStep, * orgin;

		for (uint8_t* destPixel = (uint8_t*)newSurface->buffer + 4 + yStep; destPixel < end; destPixel += 4, srcPixel += 4)
		{
			orgin = srcPixel - yStep;
			*destPixel = (*srcPixel + srcPixel[4] + *(srcPixel - 4) + srcPixel[yStep] + *orgin) / 5;
			destPixel[1] = (srcPixel[1] + srcPixel[5] + *(srcPixel - 3) + srcPixel[yStep + 1] + *(orgin + 1)) / 5;
			destPixel[2] = (srcPixel[2] + srcPixel[6] + *(srcPixel - 2) + srcPixel[yStep + 2] + *(orgin + 2)) / 5;
		}

		*newSurface >> surface;

		delete newSurface;
	}

	inline void copySurface(const Surface& src, Surface& dest)
	{
		src >> dest;
	}
	*/

	namespace Draw
	{
		void _straightLine(int d1, int d2, int s, bool slope, uint color, bool dash = false, Surface& surface = World::sScreen, int thickness = 1)
		{
			/*
			if (slope)
				d = y;
			else
				d = x;
			*/

			// Slope configurations
			Point size = surface.size;
			int step = 1;

			if (slope)
			{
				step = size.x;
				std::swap(size.x, size.y);
			}

			int sxm = size.x - 1;

			// Actual part
			if (s < 0 || s >= size.y)
				return;

			int cache = d1;
			d1 = clamp(d1, 0, sxm);
			d2 = clamp(d2, 0, sxm);

			if (d1 == d2)
				return;

			uint* startPixel, * endPixel;

			if (slope)
			{
				startPixel = getRawPixel({ s, d1 }, surface);
				endPixel = getRawPixel({ s, d2 }, surface);
			}
			else
			{
				startPixel = getRawPixel({ d1, s }, surface);
				endPixel = getRawPixel({ d2, s }, surface);
			}

			if (startPixel > endPixel)
				std::swap(startPixel, endPixel);

			if (dash)
			{
				/*
				a = World::ihCellSize

				x = x1 - old_x1;
				index = cache % a;

				if (x % 2a >= a)
					startPixel += a - index;
					index = 0;
				else
					index = x % a;

				But:
				if (x % 2a < a)
					mod(x, a) = (x % 2a)
				else
					mod(x, a) = (x % 2a) - a
				*/

				cache = (d1 - cache) % World::idCellSize;
				int index = 0;
				int dashStep = World::iCellSize * step;

				if (cache > World::iCellSize)
					startPixel += (World::idCellSize - cache) * step;
				else
					index = cache;

				while (startPixel < endPixel)
				{
					*startPixel = color;
					startPixel += step;

					if (index++ == World::iCellSize)
					{
						index = 1;
						// startPixel += World::ihCellSize * step;
						startPixel += dashStep;
					}
				}

				return;
			}

			if (thickness > 1)
			{
				int reverse_step = slope ? 1 : size.x;
				thickness = (thickness - 1) * reverse_step;

				while (startPixel < endPixel)
				{
					for (int i = -thickness; i <= thickness; i += reverse_step)
					{
						*(startPixel + i) = color;
					}
					startPixel += step;
				}

				return;
			}

			while (startPixel <= endPixel)
			{
				*startPixel = color;
				startPixel += step;
			}
		}

		inline void xLine(int x1, int x2, int y, uint color, bool dash = false, Surface& surface = World::sScreen, int thickness = 1)
		{
			_straightLine(x1, x2, y, false, color, dash, surface, thickness);
		}

		inline void yLine(int y1, int y2, int x, uint color, bool dash = false, Surface& surface = World::sScreen, int thickness = 1)
		{
			_straightLine(y1, y2, x, true, color, dash, surface, thickness);
		}

		bool straightLine(Point start, Point end, uint color, bool dash = false, Surface& surface = World::sScreen, int thickness = 1)
		{
			Point delta = end - start;

			if (!(delta.x ^ delta.y))
				return true;

			if (delta.x == 0) // Equals slope
				_straightLine(start.y, end.y, start.x, true, color, dash, surface, thickness);
			else
				_straightLine(start.x, end.x, start.y, false, color, dash, surface, thickness);

			return false;
		}

		// TODO: Make better way to check if it pixel in the screen.
		void line(Point start, Point end, uint color, Surface& surface = World::sScreen)
		{
			Point delta = end - start;

			if ((delta.x | delta.y) == 0)
				return;

			if (delta.x == 0 || delta.y == 0)
			{
				if (delta.x == 0) // Equals slope
					_straightLine(start.y, end.y, start.x, true, color, false, surface);
				else
					_straightLine(start.x, end.x, start.y, false, color, false, surface);
				return;
			}

			bool slope = abs(delta.y) > abs(delta.x);

			if (slope)
			{
				std::swap(start.x, start.y);
				std::swap(end.x, end.y);
			}

			if (start.x > end.x)
			{
				std::swap(start.x, end.x);
				std::swap(start.y, end.y);
			}

			delta = end - start;
			int error = delta.x / 2;
			Point step = { 1, getSign(delta.y) };
			uint* pixel;

			if (slope)
			{
				//start.x = clamp(start.x, 0, surface.size.y);
				//end.x = clamp(end.x, 0, surface.size.y);
				pixel = getRawPixel({ start.y, start.x }, surface);
				step.x = surface.size.x;
			}
			else
			{
				//start.x = clamp(start.x, 0, surface.size.x);
				//end.x = clamp(end.x, 0, surface.size.x);
				pixel = getRawPixel(start, surface);
				step.y *= surface.size.x;
			}

			for (int i = start.x; i < end.x; i++, pixel += step.x)
			{
				*pixel = color;
				error -= abs(delta.y);

				if (error < 0)
				{
					pixel += step.y;
					error += delta.x;
				}
			}
		}

		// Uses Data::pCache[0, 1]
		void circle(Point center, int radius, uint color, Surface& surface = World::sScreen)
		{
			Point dif = { 0, radius };
			int d = 3 - 2 * radius;

			while (dif.y >= dif.x)
			{
				setPixel({ center.x - dif.x, center.y - dif.y }, color, surface);
				setPixel({ center.x - dif.x, center.y + dif.y }, color, surface);
				setPixel({ center.x + dif.x, center.y - dif.y }, color, surface);
				setPixel({ center.x + dif.x, center.y + dif.y }, color, surface);

				setPixel({ center.x - dif.y, center.y - dif.x }, color, surface);
				setPixel({ center.x - dif.y, center.y + dif.x }, color, surface);
				setPixel({ center.x + dif.y, center.y - dif.x }, color, surface);
				setPixel({ center.x + dif.y, center.y + dif.x }, color, surface);

				if (d < 0)
					d += 4 * dif.x++ + 6;
				else
					d += 4 * (dif.x++ - dif.y--) + 10;
			}
		}

		void circle(Point center, int inner, int outer, uint color, Surface& surface = World::sScreen)
		{
			int xo = outer, xi = inner, y = 0, erro = 1 - xo, erri = 1 - xi;

			while (xo >= y)
			{
				xLine(center.x - xo, center.x - xi, center.y - y, color, false, surface);
				yLine(center.y - xo, center.y - xi, center.x - y, color, false, surface);
				xLine(center.x - xo, center.x - xi, center.y + y, color, false, surface);
				yLine(center.y - xo, center.y - xi, center.x + y, color, false, surface);

				xLine(center.x + xi, center.x + xo, center.y - y, color, false, surface);
				yLine(center.y + xi, center.y + xo, center.x - y, color, false, surface);
				xLine(center.x + xi, center.x + xo, center.y + y, color, false, surface);
				yLine(center.y + xi, center.y + xo, center.x + y, color, false, surface);

				if (erro < 0)
					erro += 2 * ++y + 1;
				else
					erro += 2 * (++y - --xo + 1);

				if (y > inner)
					xi = y;
				else if (erri < 0)
					erri += 2 * y + 1;
				else
					erri += 2 * (y - --xi + 1);
			}
		}

		inline void circle(Point center, int radius, int thickness, bool inner, uint color, Surface& surface = World::sScreen)
		{
			if (inner)
				circle(center, radius - thickness, radius, color, surface);
			else
				circle(center, radius, radius + thickness, color, surface);
		}

		void rect(const Point& start, const Point& end, uint color, bool dash = false, Surface& surface = World::sScreen)
		{
			xLine(start.x, end.x, start.y, color, dash, surface);
			xLine(start.x, end.x, end.y, color, dash, surface);
			yLine(start.y, end.y, start.x, color, dash, surface);
			yLine(start.y, end.y, end.x, color, dash, surface);
		}

		void rect(const Point& start, const Point& end, uint color, bool t, bool b, bool l, bool r, bool dash = false, Surface& surface = World::sScreen)
		{
			if (t)
				xLine(start.x, end.x, start.y, color, dash, surface);
			if (b)
				xLine(start.x, end.x, end.y, color, dash, surface);
			if (l)
				yLine(start.y, end.y, start.x, color, dash, surface);
			if (r)
				yLine(start.y, end.y, end.x, color, dash, surface);
		}

		inline void rectSize(const Point& start, const Point& size, uint color, bool dash = false, Surface& surface = World::sScreen)
		{
			rect(start, start + size, color, dash, surface);
		}

		// end (end.y row and end.x collum) is included
		void fillRect(Point start, Point end, uint color, float alpha = 1.0f, Surface& surface = World::sScreen)
		{
			clampToSurface(start, surface);
			clampToSurface(end, surface);

			swapPoint(start, end);

			uint* pixel = getRawPixel(start, surface);
			int yStep = surface.minus.x - (end.x - start.x);
			Point size = end - start + 1;

			if (alpha == 1.0f)
			{
				for (uint* yEnd = pixel + size.y * (yStep + size.x); pixel < yEnd; pixel += yStep)
					for (uint* xEnd = pixel + size.x; pixel < xEnd; pixel++)
						*pixel = color;

				return;
			}

			yStep <<= 2;
			size.x <<= 2;
			uint8_t* singlePix = (uint8_t*)pixel, * singleColor = (uint8_t*)&color;

			uint8_t intAlpha = 255 * alpha;

			// old float strategy 6ms -> 5ms new uint8_t strategy ( full screen )
			for (uint8_t* yEnd = singlePix + size.y * (yStep + size.x); singlePix < yEnd; singlePix += yStep)
			{
				for (uint8_t* xEnd = singlePix + size.x; singlePix < xEnd; singlePix += 4)
				{
					*singlePix = slide(*singlePix, *singleColor, intAlpha / 255);
					singlePix[1] = slide(singlePix[1], singleColor[1], intAlpha / 255);
					singlePix[2] = slide(singlePix[2], singleColor[2], intAlpha / 255);
				}
			}

			/*
			uint8_t* cache = (uint8_t*)&color;
			uint8_t rgb[3] = { *cache, cache[1], cache[2] };
			cache = (uint8_t*)pixel;

			yStep <<= 2;

			for (int y = start.y; y <= end.y; y++)
			{
				for (int x = start.x; x <= end.x; x++)
				{
					*cache = slide(*cache, *rgb, alpha);
					cache[1] = slide(cache[1], rgb[1], alpha);
					cache[2] = slide(cache[2], rgb[2], alpha);

					cache += 4;
				}
				cache += yStep;
			}
			*/
		}
		
		inline void fillRectSize(const Point& start, const Point& size, uint color, float alpha = 1.0f, Surface& surface = World::sScreen)
		{
			fillRect(start, start + size - 1, color, alpha, surface);
		}

		inline void triangle(const Point& a, const Point& b, const Point& c, uint color, Surface& surface = World::sScreen)
		{
			line(a, b, color);
			line(b, c, color);
			line(c, a, color);
		}
	};
};
