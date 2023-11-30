#include "Game.h"

#define _USE_MATH_DEFINES
#include <cmath>

#define iCellSize World::iCellSize
#define pCache Data::pCache
#define iCache Data::iCache

#define TWOPOWERNEG10 0.0009765625f
#define TWOPOWERNEG8 0.00390625f

#pragma comment(lib, "Winmm.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		Data::bRunning = false;
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		Data::bRunning = false;
		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
		Data::sMouse.left = true;
		return 0;
	case WM_LBUTTONUP:
		Data::sMouse.left = false;
		return 0;
	case WM_MBUTTONDOWN:
		Data::sMouse.middle = true;
		return 0;
	case WM_MBUTTONUP:
		Data::sMouse.middle = false;
		return 0;
	case WM_RBUTTONDOWN:
		Data::sMouse.right = true;
		return 0;
	case WM_RBUTTONUP:
		Data::sMouse.right = false;
		return 0;
	case WM_MOUSEWHEEL:
		if ((short)HIWORD(wParam) > 0)
		{
			if (iCellSize < 64)
				iCellSize += 2;
		}
		else if (iCellSize > 8)
			iCellSize -= 2;
		World::pCellOffset = Data::sMouse.worldPos * -iCellSize + Data::sMouse.roundScreenPos;
		generalUpdate();
		return 0;
	case WM_KEYDOWN:
		if (wParam >= '0' && wParam <= '9')
		{
			int v = wParam - '0';
			if (Events::bOBJHandGetInput == nullptr)
			{
				Events::chosenObject = (Events::ObjectIds)(v - 1);
				Events::clearChosenArea();
			}
			else
				Events::bOBJHandGetInput->value1 = 10 * Events::bOBJHandGetInput->value1 + v;
			return 0;
		}
		else if (wParam == VK_DELETE || wParam == VK_BACK)
		{
			Events::delChosenArea();
			if (Events::bOBJHandGetInput->value1 > 0)
				Events::bOBJHandGetInput->value1 /= 10;
			return 0;
		}

		switch (wParam)
		{
		case 'N':
			Events::bOBJHandGetInput->value1 = -Events::bOBJHandGetInput->value1;
			break;
		case VK_CONTROL:
			Events::keyCtrl ^= true;
			ShowCursor(Events::keyCtrl);
			return 0;
		case VK_RIGHT:
			Events::chosenObject = (Events::ObjectIds)((Events::chosenObject + 1) % Events::iObjectCount);
			Events::clearChosenArea();
			Events::bOBJHandGetInput = nullptr;
			return 0;
		case VK_LEFT:
			Events::chosenObject = (Events::ObjectIds)modulo(Events::chosenObject - 1, Events::iObjectCount);
			Events::clearChosenArea();
			Events::bOBJHandGetInput = nullptr;
			return 0;
		case 'D':
			Data::bDebug ^= true;
			return 0;
		case 'F':
			Data::bFontMode ^= true;
			return 0;
		case 'R':
			Events::rotation = (Events::rotation + 1) % 4;
			Events::rotationToTuple();
			return 0;
		}
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

constexpr Point EXTRA_SIZE = { 16, 39 };

BITMAPINFO bitmapInfo;

#pragma warning(disable: 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSW wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"v13";

	if (!RegisterClassW(&wc))
	{
		MessageBoxExW(0, L"Window registration failed!", L"Error", MB_OK, 0);
		return 0;
	}

	HWND window = CreateWindowExW(
		0,
		wc.lpszClassName,
		wc.lpszClassName,
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		ScreenSize.x + EXTRA_SIZE.x, ScreenSize.y + EXTRA_SIZE.y,
		0, 0, hInstance, 0
	);

	if (window == nullptr)
	{
		// Clean up
		UnregisterClassW(wc.lpszClassName, hInstance);
		MessageBoxExW(0, L"Window creation failed!", L"Error", MB_OK, 0);
		return 0;
	}

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = ScreenSize.x;
	bitmapInfo.bmiHeader.biHeight = ScreenSize.y;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(window);

	iCache[1] = World::pFit.x * iCellSize;
	iCache[0] = ScreenSize.x * iCellSize - iCache[1];

	if (World::init())
	{
		// Clean up
		UnregisterClassW(wc.lpszClassName, hInstance);
		ReleaseDC(window, hdc);
		MessageBoxExW(0, L"World initialization failed!", L"Error", MB_OK, 0);
		DestroyWindow(window);
		return 0;
	}

	if (Font::init())
	{
		// Clean up
		VirtualFree(World::sScreen.buffer, 0, MEM_RELEASE);
		UnregisterClassW(wc.lpszClassName, hInstance);
		ReleaseDC(window, hdc);
		MessageBoxExW(0, L"Font initialization failed!", L"Error", MB_OK, 0);
		DestroyWindow(window);
		return 0;
	}

	Gui::init();

	ShowWindow(window, nShowCmd);
	ShowCursor(FALSE);
	UpdateWindow(window);

	FILE* fp = nullptr;
	if (Data::bConsole)
	{
		AllocConsole();
		freopen_s(&fp, "CONOUT$", "w", stdout);

		if (fp == nullptr)
			return 0;
	}

	MSG message = { };
	DWORD dStartTime = 0;

	while (Data::bRunning)
	{
		Events::tickDrawCursor = true;

		dStartTime = timeGetTime();
		while (PeekMessageW(&message, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		updateMouse(window);

		Events::handleMiddle();

		updateMouse();

		Graphics::clear(Colors::WHITE);

		Update::drawGrid();

		Events::handleLeft();
		Events::drawPreview();
		Events::handleRight();

		Update::Update();

		Objects::drawWires();
		Objects::drawBatteries();

		// Draw Mouse
		if (Events::keyCtrl == false && Events::tickDrawCursor)
		{
			pCache[0] = Mouse.roundScreenPos - World::ihCellSize;
			pCache[1] = Mouse.roundScreenPos + World::ihCellSize;
			int thickness = iCellSize > 32 ? 2 : 1;
			Graphics::Draw::xLine(pCache[0].x, pCache[1].x, Mouse.roundScreenPos.y, Colors::BLACK, false, World::sScreen, thickness);
			Graphics::Draw::yLine(pCache[0].y, pCache[1].y, Mouse.roundScreenPos.x, Colors::BLACK, false, World::sScreen, thickness);
		}

		Gui::draw();

		// Draw info
		{
			Point infoStart = Gui::TOP_LEFT + Gui::TOP_LEFT_GAP * Font::FONT_SIZE, nextInfo;

			nextInfo = infoStart;
			Font::drawStringInc(nextInfo, "v12 ");
			Gui::intToCharBuffer(Objects::wires.size());
			Font::drawStringInc(nextInfo);
			nextInfo.x += Font::FONT_SIZE;
			Gui::intToCharBuffer(Objects::basicObjs.size());
			Font::drawString(nextInfo);

			// 1

			infoStart.y -= Font::D_FONT_SIZE;
			nextInfo = infoStart;

			Font::drawStringInc(nextInfo, "debug (d):");
			nextInfo.x += Font::FONT_SIZE;

			Gui::boolToString(Data::bDebug);
			Font::drawStringInc(nextInfo);
			nextInfo.x += Font::FONT_SIZE;

			if (Data::bDebug)
			{
				// 2

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;

				Font::drawStringInc(nextInfo, "delta time: ");
				Gui::intToCharBuffer(Data::iAvgPerf * 1000);
				Font::drawStringInc(nextInfo);
				nextInfo.x += Font::FONT_SIZE;

				Font::drawStringInc(nextInfo, "us (");

				// 2.1

				Gui::intToCharBuffer(Data::iPerformance);
				Font::drawStringInc(nextInfo);
				nextInfo.x += Font::FONT_SIZE;
				Font::drawStringInc(nextInfo, "ms) (tt: ");

				// 2.3

				Gui::intToCharBuffer(Data::iTopTime);
				Font::drawStringInc(nextInfo);
				nextInfo.x += Font::FONT_SIZE;
				Font::drawString(nextInfo, "ms)");

				// 3

				infoStart.y -= Font::D_FONT_SIZE;
				Font::drawString(infoStart, "show cursor (ctrl)");

				// 4

				infoStart.y -= Font::D_FONT_SIZE;
				Font::drawString(infoStart, "font test (f)");

				// 5

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "tick: ");
				Gui::intToCharBuffer(Data::iTick);
				Font::drawString(nextInfo);

				// 6

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "rotation (r): ");
				Gui::intToCharBuffer(Events::rotation);
				Font::drawString(nextInfo);

				// 6

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "cell size: ");
				Gui::intToCharBuffer(iCellSize);
				Font::drawString(nextInfo);

				// 8

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "mouse: (");
				Gui::intToCharBuffer(Mouse.worldPos.x);
				Font::drawStringInc(nextInfo);
				nextInfo.x += Font::FONT_SIZE;
				Gui::intToCharBuffer(Mouse.worldPos.y);
				Font::drawStringInc(nextInfo);
				Font::drawChar(nextInfo, ')');

				// 9

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "cell leak: ");
				Gui::boolToString(World::iqCellLeak);
				Font::drawString(nextInfo);

				// 10

				infoStart.y -= Font::D_FONT_SIZE;
				nextInfo = infoStart;
				Font::drawStringInc(nextInfo, "chosen: ");
				Gui::boolToString(Events::chosenArea);
				Font::drawStringInc(nextInfo);
				nextInfo.x += Font::FONT_SIZE;
				Gui::intToCharBuffer(Events::foundObjectTypes.size());
				Font::drawString(nextInfo);

				// 11

				infoStart.y -= Font::D_FONT_SIZE;
				Font::drawString(infoStart, "1-grab 2-del 3-copy");

				// 12

				infoStart.y -= Font::D_FONT_SIZE;
				if (Events::bOBJHandGetInput == nullptr)
					Font::drawString(infoStart, "false");
				else
				{
					nextInfo = infoStart;
					Gui::intToCharBuffer(Events::bOBJHandGetInput->id);
					Font::drawStringInc(nextInfo);
					nextInfo.x += Font::FONT_SIZE;
					Font::drawStringInc(nextInfo, "(n)");
					nextInfo.x += Font::FONT_SIZE;
					Gui::intToCharBuffer(Events::bOBJHandGetInput->value1);
					Font::drawString(nextInfo);
				}
			}
		}

		/*
		{ // Draw BIG 'A' character
			Surface* newSurface = Graphics::resizeSurface(Font::characters[0], { Data::iTick / 10 + 1, Data::iTick / 10 + 1 });
			Graphics::blitSurface(World::sScreen, newSurface, { 200, 200 });
			Graphics::Draw::rectSize({ 200, 200 }, { Data::iTick / 10 + 1, Data::iTick / 10 + 1 }, Colors::RED);
		}
		*/

		/*
		{ // Cool inf loop of screen in screen
			Graphics::Surface* newSurface = Graphics::resizeSurface(World::sScreen, { 500, 500 });
			Graphics::blitSurface(World::sScreen, newSurface, { 200, 200 }, false);
			Graphics::Draw::rectSize({ 200, 200 }, { 500, 500 }, Colors::RED);
		}
		*/

		// Update::grayScale(World::sScreen);

		StretchDIBits(
			hdc,
			0, 0, ScreenSize.x, ScreenSize.y,
			0, 0, ScreenSize.x, ScreenSize.y,
			World::sScreen.buffer, &bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);

		Data::iPerformance = timeGetTime() - dStartTime;
		if (Data::iTopTime < Data::iPerformance)
			Data::iTopTime = Data::iPerformance;
		Data::iAvgPerf = slide(Data::iAvgPerf, Data::iPerformance, TWOPOWERNEG8);
		if (Data::iPerformance < Data::targetFrameTime)
		{
			Sleep(Data::targetFrameTime - Data::iPerformance);
			Data::iDeltaTime = Data::targetFrameTime;
		}
		else
			Data::iDeltaTime = Data::iPerformance;
		Data::iTick++;
	}

	VirtualFree(World::sScreen.buffer, 0, MEM_RELEASE);
	ReleaseDC(window, hdc);
	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, hInstance);

	if (Data::bConsole)
	{
		fclose(fp);
		FreeConsole();
	}

	return 0;
}
