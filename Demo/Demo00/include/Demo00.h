#ifndef __DEMO_00_H__
#define __DEMO_00_H__

#include "Globals.h"
#include "ALog.h"

#include <string>

// Locals & Globals
static HINSTANCE l_hInstance = nullptr;
static HWND	l_hMainWnd = nullptr;
#ifdef UNICODE
#include <cwchar>
static const wchar_t* l_szClassName = L"Angelica3D v0.1.0 Demo_00 Test Class";
#else
static const std::string l_szClassName = "Angelica3D v0.1.0 Demo_00 Test Class";
#endif

static int l_iRenderWidth = RENDERWIDTH;
static int l_iRenderHeight = RENDERHEIGHT;
static bool l_bFullScreen = false;
static ALog	l_Log;

// Local Functions
static ATOM RegisterWndClass(HINSTANCE hInstance);
static bool CreateMainWnd(HINSTANCE hInstance, int nCmdShow);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif
