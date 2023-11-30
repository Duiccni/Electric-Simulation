#pragma once

#include "Events.h"

namespace Gui
{
	Point TOP_LEFT = { 0, World::sScreen.size.y - Font::FONT_SIZE };

	constexpr int GUI_FONT_GAPS = Font::H_FONT_SIZE;
	constexpr int GUI_FONT_GAPS_W_FONT = GUI_FONT_GAPS + Font::FONT_SIZE;
	constexpr int GUI_GAP = 16;

	// Uses Data::pCache[2]
	void draw()
	{
		Data::pCache[2] = { GUI_GAP, GUI_GAP };

		const char* label = nullptr;
		int sx, length = 0;

		Point start, end;

		for (int i = 0; i < Events::iObjectCount; i++)
		{
			switch (i)
			{
			case Events::oHand:
				length = OBJ_HAND_STRING_LENPX;
				label = OBJ_HAND_STRING;
				break;
			case Events::oWire:
				length = OBJ_WIRE_STRING_LENPX;
				label = OBJ_WIRE_STRING;
				break;
			case Events::oShow:
				length = OBJ_SHOW_STRING_LENPX;
				label = OBJ_SHOW_STRING;
				break;
			case Events::oBattery:
				length = OBJ_BATTERY_STRING_LENPX;
				label = OBJ_BATTERY_STRING;
				break;
			case Events::oResistor:
				length = OBJ_RESISTOR_STRING_LENPX;
				label = OBJ_RESISTOR_STRING;
				break;
			}

			sx = Data::pCache[2].x;
			start = { Data::pCache[2].x - GUI_FONT_GAPS, Data::pCache[2].y - GUI_FONT_GAPS };
			Data::pCache[2].x += length;
			end = { Data::pCache[2].x - Font::CHAR_GAP_PX + GUI_FONT_GAPS, Data::pCache[2].y + GUI_FONT_GAPS_W_FONT };

			Graphics::Draw::fillRect(start, end - 1, i == Events::chosenObject ? Colors::RED : Colors::GRAY, 0.5f);
			
			Font::drawString({ sx, Data::pCache[2].y }, label);

			if (i == Events::chosenObject)
				Graphics::Draw::rect(start, end, Colors::BLACK);

			Data::pCache[2].x += GUI_FONT_GAPS_W_FONT;
		}
	}

	void init()
	{
		stringBuffer = new char[stringBufferSize];
	}
};
