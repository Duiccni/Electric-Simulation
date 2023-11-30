#pragma once

#include "Font.h"

#define Mouse Data::sMouse

#include <list>

#define Surface Graphics::Surface

namespace Gui
{
	constexpr Point TOP_LEFT_GAP = { 1, -1 };

	constexpr int LABEL_GAP = Font::FONT_SIZE << 3;
	constexpr int LABEL_GAP3 = Font::FONT_SIZE * 3;
	constexpr int LABEL_GAP4 = Font::FONT_SIZE << 2;
	constexpr int LABEL_GAP5 = Font::FONT_SIZE * 5;
	constexpr int LABEL_GAP7 = Font::FONT_SIZE * 7;
	constexpr int LABEL_GAP11 = Font::FONT_SIZE * 11;

	constexpr int LIMIT_OF_UNSIZED_FONT = 24;

	constexpr int WIRE_DEBUG_INFO_GAP = 10;
	constexpr int WIRE_DEBUG_INFO_HGAP = WIRE_DEBUG_INFO_GAP >> 1;
	constexpr int WIRE_DEBUG_INFO_STEP = Font::FONT_SIZE + WIRE_DEBUG_INFO_HGAP;
	constexpr Point WIRE_DEBUG_INFO_SIZE = { Font::FONT_SIZE * 13 + WIRE_DEBUG_INFO_GAP, WIRE_DEBUG_INFO_STEP * 3 + WIRE_DEBUG_INFO_GAP };
	Point scaledWireDebugInfoSize = WIRE_DEBUG_INFO_SIZE;

	constexpr Point WIRE_DEBUG_LABEL1_OFFSET = { WIRE_DEBUG_INFO_HGAP, WIRE_DEBUG_INFO_HGAP };
	constexpr Point WIRE_DEBUG_LABEL2_OFFSET = { WIRE_DEBUG_INFO_HGAP, WIRE_DEBUG_INFO_HGAP + WIRE_DEBUG_INFO_STEP };
	constexpr Point WIRE_DEBUG_LABEL3_OFFSET = { WIRE_DEBUG_INFO_HGAP, WIRE_DEBUG_INFO_HGAP + WIRE_DEBUG_INFO_STEP * 2};

	constexpr int OBJ_HAND_STRING_LENPX = 4 * Font::FONT_SIZE;
	const char* OBJ_HAND_STRING = "hand";
	constexpr int OBJ_WIRE_STRING_LENPX = 4 * Font::FONT_SIZE;
	const char* OBJ_WIRE_STRING = "wire";
	constexpr int OBJ_SHOW_STRING_LENPX = 4 * Font::FONT_SIZE;
	const char* OBJ_SHOW_STRING = "show";
	constexpr int OBJ_BATTERY_STRING_LENPX = 7 * Font::FONT_SIZE;
	const char* OBJ_BATTERY_STRING = "battery";
	constexpr int OBJ_RESISTOR_STRING_LENPX = 8 * Font::FONT_SIZE;
	const char* OBJ_RESISTOR_STRING = "resistor";

	char* stringBuffer;
	constexpr int stringBufferSize = 32;

	const char* TRUE_STRING = "true";
	const char* FALSE_STRING = "false";

	inline void boolToString(bool val)
	{
		strcpy(stringBuffer, val ? TRUE_STRING : FALSE_STRING);
	}

	int intToCharBuffer(int val)
	{
		if (val == 0)
		{
			stringBuffer[0] = '0';
			stringBuffer[1] = '\0';
			return 1;
		}

		bool sign = val < 0;
		if (sign)
		{
			val = -val;
			stringBuffer[0] = '-';
		}

		int length = static_cast<int>(log10(val)) + 1 + sign;
		stringBuffer[length] = '\0';
		char* addr = stringBuffer + length - 1;

		while (val > 0)
		{
			*addr = val % 10 + '0';
			val /= 10;
			addr--;
		}

		return length;
	}
}

namespace Font
{
	void drawString(Point point, const char* str = Gui::stringBuffer, Surface& surface = World::sScreen)
	{
		while (*str != '\0')
		{
			drawChar(point, *str, surface);
			point.x += FONT_SIZE;
			str++;
		}
	}

	void drawStringInc(Point& point, const char* str = Gui::stringBuffer, Surface& surface = World::sScreen)
	{
		while (*str != '\0')
		{
			drawChar(point, *str, surface);
			point.x += FONT_SIZE;
			str++;
		}
	}
}

namespace Events
{
	int wireIdIndex = 0;
	int basicObjIdIndex = 0;
}

namespace Objects
{
	class BasicObj;

	class Wire
	{
	public:
		bool chosen = false; // IMPORTANT NOTE: this boolean value should be on top of the class EVERYTIME becouse when we are trying to get it we use an bit manipluation technique.
		int iResistance = 0;
		int iPower = 0;
		int id; // HARD const
		Point start = pZero, end = pZero; // move-dynamic
		Point wStart, wEnd; // const - (Change on move)
		bool slope, reversed = false; // HARD const

		Objects::BasicObj* anyConnectionToResistor = nullptr; // create-dynamic - (Change on move [lazy to do])

		std::list<Wire*> connectedWires; // create-dynamic - (Change on move)
		
		Wire* startWire = nullptr, * endWire = nullptr; // const-delete - (Change on move)

		Wire(const Point& start, const Point& end);
		~Wire();

		bool checkOnWorld(const Point& point) const;
		bool checkOnScreen(const Point& point) const;
		void calcScreenPoint();
		void draw(uint color);

	private:
		uint* startPixel = nullptr, * endPixel = nullptr; // move-dynamic
		bool bOutOfScreen = true; // move-dynamic
		int step; // HARD const
	};
}

namespace Events
{
	Objects::Wire* startWire = nullptr, * endWire = nullptr;
}

#define foreachWire for (Objects::Wire* wire : Objects::wires)
#define foreachBasicObj for (Objects::BasicObj* basicObj : Objects::basicObjs)
#define asm_cast(T, x) *((T*)&x)

namespace Objects
{
	bool Wire::checkOnWorld(const Point& point) const
	{
		return (point.x >= wStart.x && point.x <= wEnd.x && point.y >= wStart.y && point.y <= wEnd.y);
	};

	bool Wire::checkOnScreen(const Point& point) const
	{
		return (point.x >= start.x && point.x <= end.x && point.y >= start.y && point.y <= end.y);
	};

	void Wire::calcScreenPoint()
	{
		Graphics::worldToScreen(wStart, start, true, 1 << slope);
		Graphics::worldToScreen(wEnd, end, true, 1 << slope);

		if (slope)
		{
			bOutOfScreen = start.x < 0 || start.x >= World::sScreen.size.x;
			if (bOutOfScreen)
				return;

			start.y = clamp(start.y, 0, World::sScreen.minus.y);
			end.y = clamp(end.y, 0, World::sScreen.minus.y);

			bOutOfScreen = start.y == end.y;
			if (bOutOfScreen)
				return;
		}
		else
		{
			bOutOfScreen = start.y < 0 || start.y >= World::sScreen.size.y;
			if (bOutOfScreen)
				return;

			start.x = clamp(start.x, 0, World::sScreen.minus.x);
			end.x = clamp(end.x, 0, World::sScreen.minus.x);

			bOutOfScreen = start.x == end.x;
			if (bOutOfScreen)
				return;
		}

		startPixel = Graphics::getRawPixel(start);
		endPixel = Graphics::getRawPixel(end);
	}

	Surface wireInfoSurface = Surface(Gui::WIRE_DEBUG_INFO_SIZE);

	std::list<Wire*> wires;
	
	inline Wire* getWireAtWorld(const Point& point)
	{
		foreachWire
			if (wire->checkOnWorld(point))
				return wire;

		return nullptr;
	}

	constexpr uint8_t CHECK_START_EDGE = 0b1;
	constexpr uint8_t CHECK_END_EDGE = 0b10;
	constexpr uint8_t CHECK_BOTH_EDGES = 0b100;

	inline Wire* getWireEdgeAtWorld(const Point& point, uint8_t check = CHECK_BOTH_EDGES)
	{
		if (check & CHECK_BOTH_EDGES)
		{
			foreachWire if (wire->wStart == point || wire->wEnd == point) return wire;
		}
		else if (check & CHECK_START_EDGE)
		{
			foreachWire if (wire->wStart == point) return wire;
		}
		else if (check & CHECK_END_EDGE)
		{
			foreachWire if (wire->wEnd == point) return wire;
		}

		return nullptr;
	}

	inline Wire* getWireAtScreen(const Point& point)
	{
		foreachWire
			if (wire->checkOnScreen(point))
				return wire;

		return nullptr;
	}

	Wire::Wire(const Point& start, const Point& end)
	{
		wStart = start;
		wEnd = end;

		Graphics::straightenLine(wStart, wEnd);

		slope = wStart.x == wEnd.x;

		step = (slope ? World::sScreen.size.x : 1);

		if (wStart.y > wEnd.y)
		{
			std::swap(wStart.y, wEnd.y);
			reversed = true;
		}
		else if (wStart.x > wEnd.x)
		{
			std::swap(wStart.x, wEnd.x);
			reversed = true;
		}

		bool startConnect = Events::startWire && Events::startWire->slope == slope;

		if (startConnect || (Events::endWire && Events::endWire->slope == slope))
		{
			Wire* connWire, * otherWireConn;

			if (startConnect)
			{
				connWire = Events::startWire;
				otherWireConn = Events::endWire;
			}
			else
			{
				connWire = Events::endWire;
				otherWireConn = Events::startWire;
			}

			if (wEnd == connWire->wStart)
			{
				connWire->wStart = wStart;
				connWire->calcScreenPoint();

				connWire->startWire = otherWireConn;
				if (otherWireConn)
					otherWireConn->connectedWires.push_back(connWire);
			}
			else if (wStart == connWire->wEnd)
			{
				connWire->wEnd = wEnd;
				connWire->calcScreenPoint();

				connWire->endWire = otherWireConn;
				if (otherWireConn)
					otherWireConn->connectedWires.push_back(connWire);
			}

			Events::error = asm_cast(uint64_t, connWire);
			delete this;
			return;
		}

		wires.push_back(this);
		id = Events::wireIdIndex++;
	}

	inline void updateWires()
	{
		foreachWire wire->calcScreenPoint();
	}
	
	class BasicObj
	{
	public:
		bool chosen = false; // IMPORTANT NOTE: this boolean value should be on top of the class EVERYTIME becouse when we are trying to get it we use an bit manipluation technique.
		int value1;
		Events::ObjectIds type;

		int id;
		Point pos, wPos, wStartPos, wEndPos;
		bool slope, reversed;
		int rot;

		Wire* startWire = nullptr, * endWire = nullptr;

		BasicObj(const Point& pos, bool reversed, bool slope, Events::ObjectIds type);
		~BasicObj();

		void calcScreenPoint();
		void draw();
	private:
		bool bOutOfScreen = true;
		Point drawingCache[4];
	};

	std::list<BasicObj*> basicObjs;

	void Wire::draw(uint color)
	{
		if (bOutOfScreen)
			return;

		if (Data::bDebug)
		{
			Graphics::Draw::circle(start, World::iqCellSize, Colors::RED);

			Point infoStart = start - Gui::TOP_LEFT_GAP * World::iqCellSize;

			if (startWire && startWire->wStart == wStart)
				infoStart.x -= Gui::scaledWireDebugInfoSize.x;
			else
				infoStart.x += World::ihCellSize;

			{
				Graphics::Draw::fillRectSize(infoStart, Gui::scaledWireDebugInfoSize, Colors::GRAY, 0.125f);

				Graphics::clear(Colors::BLACK, wireInfoSurface);

				Data::pCache[2] = Gui::WIRE_DEBUG_LABEL3_OFFSET;
				Font::drawStringInc(Data::pCache[2], Gui::OBJ_WIRE_STRING, wireInfoSurface);
				Data::pCache[2].x += Font::FONT_SIZE;

				Gui::intToCharBuffer(id);
				Font::drawStringInc(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);
				Data::pCache[2].x += Font::FONT_SIZE;

				Font::drawChar(Data::pCache[2], 'x' + slope, wireInfoSurface);

				if (startWire)
					Gui::intToCharBuffer(startWire->id);
				else
					strcpy(Gui::stringBuffer, Gui::FALSE_STRING);
				Data::pCache[2] = Gui::WIRE_DEBUG_LABEL2_OFFSET;
				Font::drawStringInc(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);
				Data::pCache[2].x += Font::FONT_SIZE;

				if (endWire)
					Gui::intToCharBuffer(endWire->id);
				else
					strcpy(Gui::stringBuffer, Gui::FALSE_STRING);
				Font::drawString(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);

				Gui::intToCharBuffer(connectedWires.size());
				Data::pCache[2] = Gui::WIRE_DEBUG_LABEL1_OFFSET;
				Font::drawStringInc(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);
				Font::drawChar(Data::pCache[2], 'c', wireInfoSurface);

				Data::pCache[2].x += Font::D_FONT_SIZE;
				Gui::intToCharBuffer(iPower);
				Font::drawStringInc(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);
				Font::drawChar(Data::pCache[2], 'x', wireInfoSurface);

				Data::pCache[2].x += Font::D_FONT_SIZE;
				if (anyConnectionToResistor)
					Gui::intToCharBuffer(anyConnectionToResistor->id);
				else
					strcpy(Gui::stringBuffer, Gui::FALSE_STRING);
				Font::drawStringInc(Data::pCache[2], Gui::stringBuffer, wireInfoSurface);


				Surface* sScaledWireInfo = Graphics::resizeSurface(wireInfoSurface, Gui::scaledWireDebugInfoSize);
				Graphics::blitSurface(World::sScreen, *sScaledWireInfo, infoStart);
				delete sScaledWireInfo;
			}

			/*
			Graphics::Draw::fillRectSize(infoStart, Gui::WIRE_DEBUG_INFO_SIZE, Colors::GRAY, 0.25f);
			Graphics::Draw::rectSize(infoStart, Gui::WIRE_DEBUG_INFO_SIZE, Colors::BLACK);

			Font::drawString(infoStart + Gui::WIRE_DEBUG_LABEL3_OFFSET, Gui::OBJ_WIRE_STRING);

			Point nextStart = infoStart + Gui::WIRE_DEBUG_LABEL3_OFFSET;

			{
				Gui::intToCharBuffer(id);
				nextStart.x += Gui::LABEL_GAP5;
				Font::drawString(nextStart);

				Gui::boolToString(slope);
				nextStart.x += Gui::LABEL_GAP3;
				Font::drawString(nextStart);
			}

			nextStart = infoStart + Gui::WIRE_DEBUG_LABEL2_OFFSET;

			{
				Gui::boolToString(startWire);
				Font::drawString(nextStart);

				Gui::boolToString(endWire);
				nextStart.x += Gui::LABEL_GAP;
				Font::drawString(nextStart);
			}
			*/

			if (startWire)
				Graphics::Draw::circle(start, World::iqhCellSize, Colors::BLACK);
			if (endWire)
				Graphics::Draw::circle(end, World::iqhCellSize, Colors::BLACK);

			Graphics::Draw::rect(start - World::iqCellSize, end + World::iqCellSize, color);
			return;
		}

		uint* pixel = startPixel;

		color = chosen ? Colors::BLUE : color;

		while (pixel < endPixel)
		{
			*pixel = color;
			pixel += step;
		}

		if (startWire)
			Graphics::Draw::circle(start, World::iqhCellSize, Colors::BLACK);
		if (endWire)
			Graphics::Draw::circle(end, World::iqhCellSize, Colors::BLACK);
	}

	inline void drawWireWithConnections(Wire* wire, uint color, std::list<Wire*>& stillDont)
	{
		if (std::find(stillDont.begin(), stillDont.end(), wire) == stillDont.end())
			return;
		wire->draw(color);
		stillDont.remove(wire);
		if (wire->startWire)
			drawWireWithConnections(wire->startWire, color, stillDont);
		if (wire->endWire)
			drawWireWithConnections(wire->endWire, color, stillDont);
		for (Wire* sWire : wire->connectedWires)
			drawWireWithConnections(sWire, color, stillDont);
	}

	inline void drawWires()
	{
		if (wires.size() == 0)
			return;
		static uint color;
		int h = 0;
		std::list<Wire*> stillDont(wires);
		while (stillDont.size() > 0)
		{
			color = Graphics::HSVtoHEX(h, 255, 255);
			drawWireWithConnections(*stillDont.begin(), color, stillDont);
			h += 45;
		}
	}

	Wire::~Wire()
	{
		wires.remove(this);

		if (startWire)
			startWire->connectedWires.remove(this);
		if (endWire)
			endWire->connectedWires.remove(this);

		foreachWire
		{
			if (wire->startWire == this)
				wire->startWire = nullptr;
			else if (wire->endWire == this)
				wire->endWire = nullptr;
		}

		foreachBasicObj
		{
			if (basicObj->startWire == this)
				basicObj->startWire = nullptr;
			else if (basicObj->endWire == this)
				basicObj->endWire = nullptr;
		}
	}

	void BasicObj::calcScreenPoint()
	{
		static int s1;
		Graphics::worldToScreen(wPos, pos, true, 0);

		switch (type)
		{
		case Events::oBattery:
			bOutOfScreen = !Graphics::isInside(pos);

			if (bOutOfScreen)
				return;

			s1 = reversed ? -World::ihCellSize : World::ihCellSize;

			if (slope)
			{
				drawingCache[0] = pos;
				drawingCache[0].y -= s1;

				drawingCache[2] = pos;
				drawingCache[2].y += s1;
				drawingCache[3] = drawingCache[2];
				drawingCache[3].y += s1;

				drawingCache[1] = drawingCache[0];
				drawingCache[1].x += World::iqhCellSize;
				drawingCache[3].x = drawingCache[1].x;
				drawingCache[1].y -= s1;

				drawingCache[0].x -= World::iqhCellSize;
				drawingCache[2].x = drawingCache[0].x;
			}
			else
			{
				drawingCache[0] = pos;
				drawingCache[0].x -= s1;

				drawingCache[2] = pos;
				drawingCache[2].x += s1;
				drawingCache[3] = drawingCache[2];
				drawingCache[3].x += s1;

				drawingCache[1] = drawingCache[0];
				drawingCache[1].y += World::iqhCellSize;
				drawingCache[3].y = drawingCache[1].y;
				drawingCache[1].x -= s1;

				drawingCache[0].y -= World::iqhCellSize;
				drawingCache[2].y = drawingCache[0].y;
			}
			break;
		case Events::oResistor:
			Data::pCache[2] = pos + drawingCache[1];
			Data::pCache[3] = pos - drawingCache[1];

			bOutOfScreen = Data::pCache[2].x >= World::sScreen.size.x || Data::pCache[3].x < 0 || Data::pCache[2].y >= World::sScreen.size.y || Data::pCache[3].y < 0;
			break;
		}
	}

	void BasicObj::draw()
	{
		if (bOutOfScreen)
			return;

		char symbol = 0;

		uint color = chosen ? Colors::BLUE : Colors::DARK_BLUE;

		switch (type)
		{
		case Events::oBattery:
			if (slope)
			{
				Graphics::Draw::_straightLine(pos.x - World::ihCellSize, pos.x + World::ihCellSize, drawingCache[0].y, false, color);
				Graphics::Draw::_straightLine(pos.x - World::iCellSize, pos.x + World::iCellSize, drawingCache[2].y, false, color);
			}
			else
			{
				Graphics::Draw::_straightLine(pos.y - World::ihCellSize, pos.y + World::ihCellSize, drawingCache[0].x, true, color);
				Graphics::Draw::_straightLine(pos.y - World::iCellSize, pos.y + World::iCellSize, drawingCache[2].x, true, color);
			}

			Graphics::Draw::rect(drawingCache[0], drawingCache[1], color);
			Graphics::Draw::rect(drawingCache[2], drawingCache[3], color);

			symbol = 'v';
			break;
		case Events::oResistor:
			uint* pixel = Graphics::getRawPixel(pos) - World::iCellSize * drawingCache[0].x;
			static int xStep;

			for (int i = drawingCache[3].x; i < drawingCache[3].y; i++)
			{
				xStep = (&drawingCache[2].x)[(i >> 1) & 1];
				for (int x = 0; x < World::iqCellSize; x++)
				{
					*pixel = color;
					pixel[drawingCache[0].y] = color;
					pixel += xStep;
				}
				*pixel = color;
				pixel += (i & World::iqCellLeak) * drawingCache[0].x;
			}

			symbol = 'r';
			break;
		}

		if (startWire)
		{
			Graphics::worldToScreen(wStartPos, Data::pCache[0], true);
			Graphics::Draw::circle(Data::pCache[0], World::iqCellSize, Colors::BLACK);
		}
		if (endWire)
		{
			Graphics::worldToScreen(wEndPos, Data::pCache[0], true);
			Graphics::Draw::circle(Data::pCache[0], World::iqCellSize, Colors::BLACK);
		}

		if (World::iCellSize < Gui::LIMIT_OF_UNSIZED_FONT)
			return;

		Data::pCache[0] = pos;
		Data::pCache[0].x -= Font::H_FONT_SIZE * Gui::intToCharBuffer(value1) + Font::H_FONT_SIZE;

		if (slope)
		{
			Data::pCache[0].y -= Font::H_FONT_SIZE;
			if (type == Events::oResistor)
				Data::pCache[0].x += World::iCellSize;
		}
		else
			Data::pCache[0].y -= World::iCellSize;

		Font::drawStringInc(Data::pCache[0]);
		Font::drawChar(Data::pCache[0], symbol);
	}

	inline void updateBasicObjs()
	{
		foreachBasicObj
			basicObj->calcScreenPoint();
	}

	inline BasicObj* getBasicObjAtWorld(const Point& pos, Events::ObjectIds type)
	{
		foreachBasicObj
			if (basicObj->type == type && basicObj->wPos == pos)
				return basicObj;

		return nullptr;
	}

	inline BasicObj* getBasicObjAtWorld(const Point& pos)
	{
		foreachBasicObj
			if (basicObj->wPos == pos)
				return basicObj;

		return nullptr;
	}

	inline BasicObj* getBasicObjAtScreen(const Point& pos)
	{
		foreachBasicObj
			if (basicObj->pos == pos)
				return basicObj;

		return nullptr;
	}

	BasicObj::BasicObj(const Point& pos, bool reversed, bool slope, Events::ObjectIds type)
	{
		if (getBasicObjAtWorld(pos) != nullptr)
		{
			delete this;
			return;
		}

		this->type = type;

		wPos = pos;
		this->reversed = reversed;
		this->slope = slope;
		rot = (reversed << 1) & slope;

		this->calcScreenPoint();

		wStartPos = wPos;
		wEndPos = wPos;

		int s2 = (reversed << 1) - 1;

		*(&wStartPos.x + slope) -= s2;
		*(&wEndPos.x + slope) += s2;

		startWire = getWireEdgeAtWorld(wStartPos, CHECK_BOTH_EDGES);
		endWire = getWireEdgeAtWorld(wEndPos, CHECK_BOTH_EDGES);
		if (type == Events::oResistor)
		{
			if (startWire)
				startWire->anyConnectionToResistor = this;
			if (endWire)
				endWire->anyConnectionToResistor = this;
		}

		switch (type)
		{
		case Events::oBattery:
			value1 = 60;
			break;
		case Events::oResistor:
			value1 = 1;
			// drawingCache[1] -> s1, s2
			if (slope)
			{
				drawingCache[0].x = World::sScreen.size.x;
				drawingCache[0].y = 1;
				drawingCache[1].x = World::ihCellSize;
				drawingCache[1].y = World::iCellSize;
			}
			else
			{

				drawingCache[0].x = 1;
				drawingCache[0].y = World::sScreen.size.x;
				drawingCache[1].x = World::iCellSize;
				drawingCache[1].y = World::ihCellSize;
			}
			drawingCache[2].x = (drawingCache[0].y << 1) + drawingCache[0].x;
			drawingCache[2].y = drawingCache[0].x - (drawingCache[0].y << 1);
			drawingCache[3].x = (reversed << 1) + 1;
			drawingCache[3].y = drawingCache[3].x + 8;
			// extraData = new int[3];
			// extraData[0] = (drawingCache[0].y << 1) + drawingCache[0].x;
			// extraData[1] = drawingCache[0].x - (drawingCache[0].y << 1);
			// extraData[2] = (reversed << 1) + 1;
			break;
		}

		basicObjs.push_back(this);
		id = Events::basicObjIdIndex++;
	}

	BasicObj::~BasicObj()
	{
		if (type == Events::oResistor)
		{
			if (startWire)
				startWire->anyConnectionToResistor = nullptr;
			if (endWire)
				endWire->anyConnectionToResistor = nullptr;
		}

		// delete extraData;
		basicObjs.remove(this);
	}
	
	inline void drawBatteries()
	{
		foreachBasicObj
			basicObj->draw();
	}
}