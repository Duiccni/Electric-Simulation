#pragma once

#include "Gui.h"

namespace Update
{
	void drawGrid()
	{
		uint* pixel = Graphics::getRawPixel(World::pCellOffsetMod), * endPixel;

		while (pixel < World::sScreen.end)
		{
			endPixel = pixel + Data::iCache[1];
			while (pixel < endPixel)
			{
				*pixel = Colors::BLACK;
				pixel += World::iCellSize;
			}
			*pixel = Colors::BLACK;
			pixel += Data::iCache[0];
		}
	}

	void grayScale(Surface& surface)
	{
		uint8_t* end = (uint8_t*)surface.end, color;
		for (uint8_t* pixel = (uint8_t*)surface.buffer; pixel < end; pixel += 4)
		{
			color = (*pixel + pixel[1] + pixel[2]) / 3;
			*pixel = color;
			pixel[1] = color;
			pixel[2] = color;
		}
	}

	std::list<Objects::BasicObj*> conResistors1, conResistors2;
	std::list<Objects::Wire*> stillDont;

	void updateWireRecall(Objects::Wire* wire, int iPower, bool listNum)
	{
		if (std::find(stillDont.begin(), stillDont.end(), wire) == stillDont.end())
			return;
		wire->iPower = iPower;
		stillDont.remove(wire);
		if (wire->startWire)
			updateWireRecall(wire->startWire, iPower, listNum);
		if (wire->endWire)
			updateWireRecall(wire->endWire, iPower, listNum);
		for (Objects::Wire* sWire : wire->connectedWires)
			updateWireRecall(sWire, iPower, listNum);
		if (wire->anyConnectionToResistor)
		{
			if (listNum)
				conResistors2.push_back(wire->anyConnectionToResistor);
			else
				conResistors1.push_back(wire->anyConnectionToResistor);
		}
	}

	bool onePartOfResistorCalcLoop(bool revListNum, int iPower)
	{
		bool listNum = !revListNum;
		std::list<Objects::BasicObj*>& resList = listNum ? conResistors2 : conResistors1;

		if (resList.size() == 0)
			return false;

		for (Objects::BasicObj* resistor : resList)
		{
			if (resistor->startWire != nullptr && resistor->endWire != nullptr)
			{
				updateWireRecall(resistor->startWire, iPower, revListNum);
				updateWireRecall(resistor->endWire, iPower, revListNum);
			}
		}
		resList.clear();

		return true;
	}

	void Update()
	{
		Objects::BasicObj* firstBattery = nullptr;
		foreachBasicObj
		{
			if (basicObj->type == Events::oBattery) {
				firstBattery = basicObj;
				break;
			}
		}
		if (firstBattery == nullptr)
			return;

		Objects::Wire* wire = firstBattery->startWire;
		if (wire == nullptr)
			return;

		std::copy(Objects::wires.begin(), Objects::wires.end(), std::back_inserter(stillDont));

		updateWireRecall(wire, 0, false);
		for (int i = 1; onePartOfResistorCalcLoop(i & true, i); i++);
	}
}