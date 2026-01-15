#ifndef __RENDER_H__
#define __RENDER_H__

class CRender
{
public:
    CRender();
    ~CRender();

    bool Init(HINSTANCE hInst, HWND hWnd, bool bFullScreen);  // Initialize game
    void Release();                                           // Release game

    bool ResizeDevice(int iWidth, int iHeight);

private:
    HINSTANCE m_hInstance;
    HWND      m_hRenderWnd;
    int       m_iRenderWidth;
    int       m_iRenderHeight;
    bool      m_bFullScreen;
};

extern CRender g_Render;

#endif
