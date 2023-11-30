#pragma once

#include "Graphics.h"

#include <fstream>

namespace Font
{
	const char* PATH = "font.bin";

	constexpr int FONT_SIZE = 10;
	constexpr int CHAR_GAP_PX = 2;
	constexpr int FONT_SIZE_WITHOUT_GAP = FONT_SIZE - CHAR_GAP_PX;
	constexpr int FONT_SIZE_WITH_DOUBLE_GAP = FONT_SIZE + CHAR_GAP_PX;

	int scaledWorldFontSize = FONT_SIZE;

	constexpr int H_FONT_SIZE = FONT_SIZE >> 1;
	constexpr Point POINT_FONT_SIZE = { FONT_SIZE, FONT_SIZE };
	constexpr int D_FONT_SIZE = FONT_SIZE << 1;

	constexpr int ALPHABET_SIZE = 26;
	constexpr int NUMBERS_SIZE = 10;
	constexpr int NUMBERS_OFFSET = ALPHABET_SIZE - '0';
	constexpr int SYMBOLS_SIZE = 5;
	constexpr int BEFORE_SYMBOLS = ALPHABET_SIZE + NUMBERS_SIZE;

	constexpr int FC_MINUS = BEFORE_SYMBOLS;
	constexpr int FC_PLUS = BEFORE_SYMBOLS + 1;
	constexpr int FC_BRACKET_LEFT = BEFORE_SYMBOLS + 2;
	constexpr int FC_BRACKET_RIGHT = BEFORE_SYMBOLS + 3;
	constexpr int FC_DOUBLE_DOT = BEFORE_SYMBOLS + 4;

	constexpr int TOTAL_SIZE = BEFORE_SYMBOLS + SYMBOLS_SIZE;

	constexpr int BUFFER_SIZE = FONT_SIZE * FONT_SIZE * TOTAL_SIZE;
	constexpr int Y_STEP = FONT_SIZE * (TOTAL_SIZE - 1);
	constexpr int FONT_WIDTH = FONT_SIZE * TOTAL_SIZE;

	Graphics::Surface* characters;

	char* binaryBuffer;

	bool init(uint color = Colors::VISIBLE_BLACK)
	{
		binaryBuffer = new char[BUFFER_SIZE];
		std::ifstream file(PATH, std::ios::binary);
		if (!file.is_open())
			return true;
		file.read(binaryBuffer, BUFFER_SIZE);
		file.close();

		characters = new Graphics::Surface[TOTAL_SIZE];

		for (int i = 0; i < TOTAL_SIZE; i++)
		{
			char* binaryPixel = binaryBuffer + i * FONT_SIZE;
			uint* pixel = characters[i].buffer;

			for (int y = 0; y < FONT_SIZE; y++)
			{
				for (int x = 0; x < FONT_SIZE; x++)
				{
					if (*binaryPixel == 1)
						*pixel = color;
					else
						*pixel = Colors::BLACK;
					pixel++;
					binaryPixel++;
				}
				binaryPixel += Y_STEP;
			}
		}

		delete[BUFFER_SIZE] binaryBuffer;

		return false;
	}

	int _getCharIndex(char c)
	{
		switch (c)
		{
		case ' ':
			return -1;
		case '-':
			return FC_MINUS;
		case '+':
			return FC_PLUS;
		case '(':
			return FC_BRACKET_LEFT;
		case ')':
			return FC_BRACKET_RIGHT;
		case ':':
			return FC_DOUBLE_DOT;
		}

		if (c <= '9')
			return c + NUMBERS_OFFSET;

		return c - 'a';
	}

	// Uses Data::pCache[0, 1]
	void drawChar(const Point& point, char c, Graphics::Surface& surface = World::sScreen)
	{
		int index = _getCharIndex(c);
		if (index == -1)
			return;

		if (Data::bFontMode)
		{
			Graphics::Draw::fillRectSize(point, POINT_FONT_SIZE, Colors::VISIBLE_BLACK, 1.0f, surface);
			return;
		}

		Graphics::blitSurface(surface, characters[index], point);
		return;
	}

	void drawString(Point point, const char* str, Graphics::Surface& surface);
};