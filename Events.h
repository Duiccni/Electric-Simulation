#pragma once

#include "Objects.h"

Objects::Wire* lastWire;
Objects::BasicObj* lastBasicObj;

#define ScreenSize World::sScreen.size

#include <iterator>

namespace Events
{
	enum EventIds
	{
		eNull,
		eMouseLeftDown,
		eMoving,
		eMouseRightDown,
	};

	bool keyCtrl = false, tickDrawCursor = true;
	Objects::BasicObj* bOBJHandGetInput = nullptr;
	int rotation = 0;
	bool slope, reversed;
	constexpr int iObjectCount = 5;
	EventIds eventId = eNull;
	ObjectIds chosenObject = oHand;
	Point pEventStart, pEventEnd, pEventStartScreen, pEventEndScreen;

	Point wLastChosenAreaStart, wLastChosenAreaEnd;

	bool chosenArea = false;
	bool isChosenAreaZero = true;
	std::list<uint64_t> foundObjects;
	std::list<ObjectIds> foundObjectTypes;

	inline void createWire(const Point& start, const Point& end)
	{
		lastWire = new Objects::Wire(start, end);
		lastWire->calcScreenPoint();
	}

	inline void rotationToTuple()
	{
		reversed = rotation >> 1;
		slope = rotation & 1;
	}

	void createWire()
	{
		if (pEventStart == pEventEnd)
			return;

		lastWire = new Objects::Wire(pEventStart, pEventEnd);

		if (Events::error != 0)
			lastWire = asm_cast(Objects::Wire*, Events::error);

		foreachBasicObj
		{
			if (basicObj->startWire == nullptr && (basicObj->wStartPos == lastWire->wStart || basicObj->wStartPos == lastWire->wStart))
			{
				basicObj->startWire = lastWire;
				if (basicObj->type == oResistor)
					lastWire->anyConnectionToResistor = basicObj;
			}
			else if (basicObj->endWire == nullptr && (basicObj->wEndPos == lastWire->wEnd || basicObj->wEndPos == lastWire->wEnd))
			{
				basicObj->endWire = lastWire;
				if (basicObj->type == oResistor)
					lastWire->anyConnectionToResistor = basicObj;
			}
		}

		if (Events::error != 0)
		{
			Events::error = 0;
			return;
		}

		lastWire->calcScreenPoint();

		if (lastWire->reversed)
		{
			lastWire->startWire = endWire;
			lastWire->endWire = startWire;
		}
		else
		{
			lastWire->startWire = startWire;
			lastWire->endWire = endWire;
		}

		if (startWire != nullptr)
			startWire->connectedWires.push_back(lastWire);
		if (endWire != nullptr)
			endWire->connectedWires.push_back(lastWire);
	}

	void drawWirePreview()
	{
		if (Graphics::Draw::straightLine(pEventStartScreen, pEventEndScreen, Colors::RED))
			return;

		if (startWire != nullptr)
			Graphics::Draw::circle(pEventStartScreen, World::iqhCellSize, Colors::RED);
		if (endWire != nullptr)
			Graphics::Draw::circle(pEventEndScreen, World::iqhCellSize, Colors::RED);
	}

	void drawChosenArea(uint color)
	{
		Graphics::worldToScreen(pEventStart, Data::pCache[0], true);
		Data::pCache[1] = Data::sMouse.roundScreenPos;

		isChosenAreaZero = Data::pCache[0] == Data::pCache[1];

		if (isChosenAreaZero)
			return;

		int ihSize = Data::pCache[0].x > Data::pCache[1].x ? World::ihCellSize : -World::ihCellSize;
		Data::pCache[0].x += ihSize;
		Data::pCache[1].x -= ihSize;

		ihSize = Data::pCache[0].y > Data::pCache[1].y ? World::ihCellSize : -World::ihCellSize;
		Data::pCache[0].y += ihSize;
		Data::pCache[1].y -= ihSize;

		Graphics::Draw::fillRect(Data::pCache[0], Data::pCache[1], color, 0.5f);
	}

	void clearChosenArea()
	{
		if (chosenArea == false)
			return;
		chosenArea = false;

		for (uint64_t object : foundObjects)
			*asm_cast(bool*, object) = false;

		foundObjects.clear();
		foundObjectTypes.clear();
	}

	bool isInsideTouch(Point pos, Point s, Point e)
	{
		return (
			pos.x <= e.x && pos.x >= s.x
			&& pos.y <= e.y && pos.y >= s.y
		);
	}

	inline void calcChosenArea()
	{
		if (isChosenAreaZero)
			return;

		swapPoint(pEventStart, pEventEnd);

		wLastChosenAreaStart = pEventStart;
		wLastChosenAreaEnd = pEventEnd;

		foreachBasicObj
		{
			if (isInsideTouch(basicObj->wPos, pEventStart, pEventEnd))
			{
				foundObjects.push_back(asm_cast(uint64_t, basicObj));
				basicObj->chosen = true;
				foundObjectTypes.push_back(basicObj->type);
			}
		}

		foreachWire
		{
			if (isInsideTouch(wire->wStart, pEventStart, pEventEnd) || isInsideTouch(wire->wEnd, pEventStart, pEventEnd))
			{
				foundObjects.push_back(asm_cast(uint64_t, wire));
				wire->chosen = true;
				foundObjectTypes.push_back(oWire);
			}
		}

		chosenArea = foundObjectTypes.size() > 0;
	}

	bool isNotWithUs(Objects::Wire* wire, int length)
	{
		bool isNotWithUs = true;

		auto itOS = foundObjects.begin();
		auto itTS = foundObjectTypes.begin();
		int iS = 0;
		while (true)
		{
			if (*itTS == oWire && wire == asm_cast(Objects::Wire*, *itOS))
			{
				isNotWithUs = false;
				break;
			}

			if (++iS >= length)
				break;

			itOS = std::next(itOS);
			itTS = std::next(itTS);
		}

		return isNotWithUs;
	}

	inline void moveChosenArea()
	{
		if (chosenArea == false)
			return;

		int length = foundObjectTypes.size();
		static Objects::Wire* wire;
		static Objects::BasicObj* basicObj;
		Point diff = pEventEnd - pEventStart;

		bool isWire = false;

		auto itO = foundObjects.begin();
		auto itT = foundObjectTypes.begin();
		int i = 0;
		while (true)
		{
			isWire = (*itT == oWire);
			if (isWire)
				wire = (Objects::Wire*)*itO;
			else
				basicObj = (Objects::BasicObj*)*itO;

			if (isWire)
			{
				wire->wStart += diff;
				wire->wEnd += diff;

				wire->calcScreenPoint();

				if (wire->startWire && isNotWithUs(wire->startWire, length))
				{
					wire->startWire->connectedWires.remove(wire);
					wire->startWire = nullptr;
				}
				if (wire->endWire && isNotWithUs(wire->endWire, length))
				{
					wire->endWire->connectedWires.remove(wire);
					wire->endWire = nullptr;
				}

				auto itCW = wire->connectedWires.begin();
				int i2 = 0;
				int len2 = wire->connectedWires.size();
				while (i < len2)
				{
					Objects::Wire* sWire = *itCW;
					itCW = std::next(itCW);
					if (isNotWithUs(*itCW, length))
					{
						if (sWire->startWire == wire)
							sWire->startWire = nullptr;
						else
							sWire->endWire = nullptr;
						sWire->connectedWires.remove(wire);
					}

					i++;
				}
			}
			/*
			else
			{
				// TODO
			}
			*/

			if (++i >= length)
				break;

			itO = std::next(itO);
			itT = std::next(itT);
		}

		clearChosenArea();
	}

	void handleLeft()
	{
		if (Data::sMouse.left)
		{
			if (eventId == eNull)
			{
				eventId = eMouseLeftDown;
				pEventStart = Data::sMouse.worldPos;
				pEventStartScreen = Data::sMouse.roundScreenPos;
				pEventEndScreen = Data::sMouse.roundScreenPos;

				if (chosenObject == oHand)
				{
					wLastChosenAreaStart -= pEventStart;
					wLastChosenAreaEnd -= pEventStart;
					pEventEnd = Data::sMouse.worldPos;
				}
				else
				{
					startWire = Objects::getWireAtWorld(pEventStart);
					endWire = nullptr;
				}
			}
			else if (eventId != eMouseLeftDown)
				return;

			switch (chosenObject)
			{
			case Events::oWire:
				drawWirePreview();
				break;
			case oHand:
				if (chosenArea == false)
					break;

				Graphics::Draw::line(pEventStartScreen, pEventEndScreen, Colors::BLACK);
				Graphics::worldToScreen(wLastChosenAreaStart + pEventEnd, Data::pCache[0], true, Graphics::CLAMP_BOTH);
				Graphics::worldToScreen(wLastChosenAreaEnd + pEventEnd, Data::pCache[1], true, Graphics::CLAMP_BOTH);
				Graphics::Draw::fillRect(Data::pCache[0], Data::pCache[1], Colors::DARK_BLUE, 0.5f);
				break;
			}
		}
		else if (eventId == eMouseLeftDown)
		{
			pEventEnd = Data::sMouse.worldPos;

			switch (chosenObject)
			{
			case oHand:
				moveChosenArea();
				break;
			case Events::oWire:
				createWire();
				break;
			case oBattery:
				lastBasicObj = new Objects::BasicObj(Mouse.worldPos, reversed, slope, oBattery);
				break;
			case oResistor:
				lastBasicObj = new Objects::BasicObj(Mouse.worldPos, reversed, slope, oResistor);
				break;
			}

			eventId = eNull;
		}
	}

	// pCache[0, 1, 2, 3]
	void drawPreview()
	{
		static int s1;

		if (eventId == eNull)
		{
			switch (chosenObject)
			{
			case Events::oBattery:
				tickDrawCursor = false;
				s1 = reversed ? -World::ihCellSize : World::ihCellSize;
				if (slope)
				{
					// pCache[0, 1, 2, 3]
					Data::pCache[0] = Mouse.roundScreenPos;
					Data::pCache[0].y -= s1;

					Data::pCache[2] = Mouse.roundScreenPos;
					Data::pCache[2].y += s1;
					Data::pCache[3] = Data::pCache[2];
					Data::pCache[3].y += s1;

					Data::pCache[1] = Data::pCache[0];
					Data::pCache[1].x += World::iqhCellSize;
					Data::pCache[3].x = Data::pCache[1].x;
					Data::pCache[1].y -= s1;

					Data::pCache[0].x -= World::iqhCellSize;
					Data::pCache[2].x = Data::pCache[0].x;

					Graphics::Draw::_straightLine(Mouse.roundScreenPos.x - World::ihCellSize, Mouse.roundScreenPos.x + World::ihCellSize, Data::pCache[0].y, false, Colors::DARK_BLUE);
					Graphics::Draw::_straightLine(Mouse.roundScreenPos.x - World::iCellSize, Mouse.roundScreenPos.x + World::iCellSize, Data::pCache[2].y, false, Colors::DARK_BLUE);
				}
				else
				{
					// pCache[0, 1, 2, 3]
					Data::pCache[0] = Mouse.roundScreenPos;
					Data::pCache[0].x -= s1;

					Data::pCache[2] = Mouse.roundScreenPos;
					Data::pCache[2].x += s1;
					Data::pCache[3] = Data::pCache[2];
					Data::pCache[3].x += s1;

					Data::pCache[1] = Data::pCache[0];
					Data::pCache[1].y += World::iqhCellSize;
					Data::pCache[3].y = Data::pCache[1].y;
					Data::pCache[1].x -= s1;

					Data::pCache[0].y -= World::iqhCellSize;
					Data::pCache[2].y = Data::pCache[0].y;

					Graphics::Draw::_straightLine(Mouse.roundScreenPos.y - World::ihCellSize, Mouse.roundScreenPos.y + World::ihCellSize, Data::pCache[0].x, true, Colors::DARK_BLUE);
					Graphics::Draw::_straightLine(Mouse.roundScreenPos.y - World::iCellSize, Mouse.roundScreenPos.y + World::iCellSize, Data::pCache[2].x, true, Colors::DARK_BLUE);

					/*
					Data::pCache[4] = Mouse.roundScreenPos;
					Data::pCache[4].x -= World::iCellSize;
					Data::pCache[4].y += World::ihCellSize - Font::H_FONT_SIZE;

					Font::drawChar(Data::pCache[4], '-');
					Data::pCache[4].x += World::idCellSize - Font::FONT_SIZE_WITHOUT_GAP;
					Font::drawChar(Data::pCache[4], '+');
					*/
				}

				Graphics::Draw::rect(Data::pCache[0], Data::pCache[1], Colors::DARK_BLUE);
				Graphics::Draw::rect(Data::pCache[2], Data::pCache[3], Colors::DARK_BLUE);
				break;
			case Events::oResistor:
				tickDrawCursor = false;

				static int x, y, s1, s2;

				if (slope)
				{
					x = ScreenSize.x;
					y = 1;
					s1 = World::ihCellSize;
					s2 = World::iCellSize;
				}
				else
				{
					y = ScreenSize.x;
					x = 1;
					s1 = World::iCellSize;
					s2 = World::ihCellSize;
				}

				if (Mouse.roundScreenPos.x + s1 >= ScreenSize.x || Mouse.roundScreenPos.x - s1 < 0)
					return;

				if (Mouse.roundScreenPos.y + s2 >= ScreenSize.y || Mouse.roundScreenPos.y - s2 < 0)
					return;

				uint* pixel = Mouse.roundScreenPixel - World::iCellSize * x;
				static int xStep;

				int v[2] = { (y << 1) + x, x - (y << 1) };
				int iEnd = 9 + (reversed << 1);

				for (int i = (reversed << 1) + 1; i < iEnd; i++)
				{
					xStep = v[(i >> 1) & 1];
					for (int x = 0; x < World::iqCellSize; x++)
					{
						*pixel = Colors::DARK_BLUE;
						pixel[y] = Colors::DARK_BLUE;
						pixel += xStep;
					}
					*pixel = Colors::DARK_BLUE;
					pixel += (i & World::iqCellLeak) * x;
				}

				break;
			}
		}
	}

	void endConnectedWireUpdate()
	{
		if (eventId != eMouseLeftDown)
			return;

		if (chosenObject == oWire)
		{
			pEventEndScreen = Data::sMouse.roundScreenPos;
			Graphics::straightenLine(pEventStartScreen, pEventEndScreen);

			endWire = Objects::getWireAtScreen(pEventEndScreen);
			if (endWire == startWire)
				endWire = nullptr;
			return;
		}

		if (chosenObject == oHand)
		{
			pEventEndScreen = Data::sMouse.roundScreenPos;
			pEventEnd = Data::sMouse.worldPos;
			Data::pReserveCache[0] = Data::sMouse.roundScreenPos;
			Graphics::straightenLine(pEventStartScreen, Data::pReserveCache[0]);
		}
	}
	
	void handleRight()
	{
		if (Data::sMouse.right)
		{
			if (eventId == eNull)
			{
				clearChosenArea();
				eventId = eMouseRightDown;
				pEventStart = Data::sMouse.worldPos;
				pEventStartScreen = Data::sMouse.roundScreenPos;
			}
			else if (eventId != eMouseRightDown)
				return;

			switch (chosenObject)
			{
			case Events::oHand:
				drawChosenArea(Colors::DARK_BLUE);
				break;
			}
		}
		else if (eventId == eMouseRightDown)
		{
			pEventEnd = Data::sMouse.worldPos;
			pEventEndScreen = Data::sMouse.roundScreenPos;

			switch (chosenObject)
			{
			case Events::oHand:
				calcChosenArea();
				break;
			case Events::oShow:
				bOBJHandGetInput = Objects::getBasicObjAtWorld(pEventEnd);
				break;
			case Events::oWire:
				delete Objects::getWireAtWorld(pEventEnd);
				break;
			case Events::oBattery:
				delete Objects::getBasicObjAtWorld(pEventEnd, Events::oBattery);
				break;
			case Events::oResistor:
				delete Objects::getBasicObjAtWorld(pEventEnd, Events::oResistor);
				break;
			}

			eventId = eNull;
		}

		// Draw Chosen Area
		if (Data::bDebug)
		{
			uint8_t data = ~(Graphics::worldToScreenWithData(pEventStart, Data::pCache[0]) | Graphics::worldToScreenWithData(pEventEnd, Data::pCache[1]));

			Graphics::Draw::rect(Data::pCache[0], Data::pCache[1], Colors::BLACK,
				data & Graphics::TOP_OUTSIDE,
				data & Graphics::BOTTOM_OUTSIDE,
				data & Graphics::LEFT_OUTSIDE,
				data & Graphics::RIGHT_OUTSIDE,
				true
			);
		}
	}

	void delChosenArea()
	{
		if (chosenArea == false)
			return;

		int length = foundObjectTypes.size();
		auto itO = foundObjects.begin();
		auto itT = foundObjectTypes.begin();
		int i = 0;
		while (true)
		{
			switch (*itT)
			{
			case oBattery:
				delete (Objects::BasicObj*)*itO;
				break;
			case oResistor:
				delete (Objects::BasicObj*)*itO;
				break;
			case oWire:
				delete (Objects::Wire*)*itO;
				break;
			}

			if (++i >= length)
				break;

			itO = std::next(itO);
			itT = std::next(itT);
		}

		clearChosenArea();
	}
};

void generalUpdate()
{
	World::ihCellSize = World::iCellSize >> 1;
	World::iqCellSize = World::iCellSize >> 2;

	World::iqCellLeak = World::ihCellSize & 1;

	World::iqhCellSize = World::iCellSize >> 3;
	World::idCellSize = World::iCellSize << 1;

	World::pCellOffsetMod = { modulo(World::pCellOffset.x, World::iCellSize), modulo(World::pCellOffset.y, World::iCellSize) };
	World::pFit = (World::sScreen.size - World::pCellOffsetMod) / World::iCellSize;
	Data::iCache[1] = World::pFit.x * World::iCellSize;
	Data::iCache[0] = World::sScreen.size.x * World::iCellSize - Data::iCache[1];

	Font::scaledWorldFontSize = Font::FONT_SIZE * World::iCellSize / World::DEFAULT_CELL_SIZE;
	Gui::scaledWireDebugInfoSize = Gui::WIRE_DEBUG_INFO_SIZE * World::iCellSize / World::DEFAULT_CELL_SIZE;

	if (Events::eventId != Events::eNull)
		Graphics::worldToScreen(Events::pEventStart, Events::pEventStartScreen, true);

	Objects::updateWires();
	Objects::updateBasicObjs();
}

void updateMouse(HWND hwnd)
{
	GetCursorPos(&Data::sMouse.winPos);
	ScreenToClient(hwnd, &Data::sMouse.winPos);
	Data::sMouse.oldPos = Data::sMouse.pos;
	Data::sMouse.pos = { Data::sMouse.winPos.x, World::sScreen.size.y - Data::sMouse.winPos.y };
	Data::sMouse.delta = Data::sMouse.pos - Data::sMouse.oldPos;
}

void updateMouse()
{
	Data::sMouse.oldWorldPos = Data::sMouse.worldPos;
	Graphics::screenToWorld(Data::sMouse.pos, Data::sMouse.worldPos);
	Data::sMouse.worldDelta = Data::sMouse.worldPos - Data::sMouse.oldWorldPos;
	Mouse.worldInScreen = Graphics::worldToScreen(Data::sMouse.worldPos, Data::sMouse.roundScreenPos) != nullptr;

	if (Mouse.worldInScreen)
	{
		if (Mouse.delta != pZero)
		{
			Data::sMouse.roundScreenPixel = Graphics::getRawPixel(Data::sMouse.roundScreenPos);
			Events::endConnectedWireUpdate();
		}
		else if (Mouse.worldDelta != pZero)
			Events::endConnectedWireUpdate();
	}
}

namespace Events
{
	void handleMiddle()
	{
		if (Data::sMouse.middle)
		{
			if (eventId == eNull)
				eventId = eMoving;
			else if (eventId != eMoving)
				return;

			if (Data::sMouse.delta != pZero)
			{
				World::pCellOffset += Data::sMouse.delta;
				generalUpdate();
			}
		}
		else if (eventId == eMoving)
			eventId = eNull;
	}
};