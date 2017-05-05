
/*

  $Id$
  Copyright (c) 2005, Alexey Suda-Chen aka hedgehog, Teggo FF

*/

#include "_common.h"
#include <libpict.h>

static HWND splashScreenWnd = 0;
static DWORD splashThread = 0;
static HBITMAP splashscreen_bmp;

enum { SPLASH_MINTIME = 2000 };

LRESULT __stdcall Splash_Screen_WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_PAINT)
	{
		PAINTSTRUCT pss;
		BITMAP bmp = {0};
		HDC bmpDC;
		HGDIOBJ oO;
		GetObject(splashscreen_bmp, sizeof(bmp) , &bmp);
		BeginPaint(hWnd, &pss);
		bmpDC = CreateCompatibleDC(pss.hdc);
		oO = SelectObject(bmpDC, splashscreen_bmp);
		BitBlt(pss.hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
		SelectObject(bmpDC, oO);
		DeleteDC(bmpDC);
		EndPaint(hWnd, &pss);
		return 0;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

BOOL CALLBACK FindOtherProcessWindow(HWND hwnd, LPARAM lparam)
{
	DWORD process = 0;
	GetWindowThreadProcessId(hwnd, &process);
	if (process == GetCurrentProcessId() && hwnd != splashScreenWnd)
	{
		ShowWindow(splashScreenWnd, SW_HIDE);
		SetFocus(hwnd);
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		PostMessageA(splashScreenWnd, WM_QUIT, 0, 0);
		splashScreenWnd = 0;
		return FALSE;
	}
	else
		return TRUE;
}

DWORD __stdcall SplashScreen_Thread(HBITMAP hBmp)
{
	static WNDCLASSEXA wndcls = {0};
	BITMAP bmp = {0};
	int i;
	int scrwidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int scrheight = GetSystemMetrics(SM_CYFULLSCREEN);
	int startms;
	intptr_t timerid;
	RECT rect;
	DWORD style = WS_POPUP | WS_DISABLED | WS_BORDER | WS_VISIBLE;
	HWND hWnd;
	MSG msg;

	splashscreen_bmp = hBmp;
	GetObject(hBmp, sizeof(bmp) , &bmp);
	wndcls.cbSize = sizeof(WNDCLASSEXA);
	wndcls.lpfnWndProc = &Splash_Screen_WndProc;
	wndcls.lpszClassName = "{CC7574E4-5E39-4700-B286-269A82DD8E95}";
	wndcls.hInstance = GetModuleHandleA(0);
	RegisterClassEx(&wndcls);

	rect.left = (scrwidth - bmp.bmWidth) / 2;
	rect.top  = (scrheight - bmp.bmHeight) / 2;
	rect.right =  rect.left + bmp.bmWidth;
	rect.bottom =  rect.top + bmp.bmHeight;

	AdjustWindowRectEx(&rect, style, 0, 0);

	hWnd = CreateWindowEx(
	           0/*WS_EX_TOPMOST*//*|WS_EX_NOACTIVATE*/,
	           "{CC7574E4-5E39-4700-B286-269A82DD8E95}", "",
	           style,
	           rect.left, rect.top,
	           rect.right - rect.left,
	           rect.bottom - rect.top,
	           0, 0, 0, 0);

	timerid = SetTimer(hWnd, 0, 100, 0);
	splashScreenWnd = hWnd;
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);

	startms = timeGetTime();

	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.hwnd == hWnd && msg.message == WM_TIMER && (startms + SPLASH_MINTIME) < timeGetTime())
		{
			for (i = 0; i < 100; ++i)
			{
				EnumWindows(FindOtherProcessWindow, 0);
				if (!splashScreenWnd) break;
			}
		}
	}

	KillTimer(hWnd, timerid);
	DestroyWindow(hWnd);
	splashScreenWnd = 0;
	DeleteObject(hBmp);

	return 0;
}

void Execute_Splash_Thread(PICTURE* pict)
{
	HBITMAP bmp = 0;
	if (!pict) return;
	bmp = Create_HBITMAP(pict);
	CreateThread(0, 0,
	(LPTHREAD_START_ROUTINE)SplashScreen_Thread,
	bmp,
	0, &splashThread);
	Sleep(111);
}

PICTURE* Pict_From_BMP_Specific_NoX(void* bytes, int count, int format)
{
	static PICTURE pict = {0,};
	if ( PICTURE_ERR_OK == Picture_From_BMP(&pict, bytes, count, format) )
		return &pict;
	else
		return NULL;
}
