#include <Windows.h>

#include "Render.h"
#include "Globals.h"

CRender	g_Render;

CRender::CRender()
    : m_hInstance(nullptr)
    , m_hRenderWnd(nullptr)
    , m_iRenderWidth(RENDERWIDTH)
    , m_iRenderHeight(RENDERHEIGHT)
    , m_bFullScreen(false)
{
}

CRender::~CRender() = default;

bool CRender::Init(HINSTANCE hInst, HWND hWnd, bool bFullScreen)
{
    m_hInstance = hInst;
    m_hRenderWnd = hWnd;
    m_bFullScreen = bFullScreen;

    return true;
}

void CRender::Release()
{
}

bool CRender::ResizeDevice(int iWidth, int iHeight)
{
    m_iRenderWidth = iWidth;
    m_iRenderHeight = iHeight;

    return true;
}
