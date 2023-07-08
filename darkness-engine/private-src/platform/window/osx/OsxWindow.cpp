#include "platform/window/osx/OsxWindow.h"
#include "tools/Debug.h"

namespace platform
{
    namespace implementation
    {
        //LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        WindowImpl::WindowImpl(void* handle, int width, int height)
            : m_windowHandle{ handle }
            , m_needCleanup{ false }
            , m_width(width)
            , m_height(height)
        {
        }

        WindowImpl::WindowImpl(const WindowImpl& impl)
        {
            ASSERT(!impl.m_needCleanup);
            m_windowHandle = impl.m_windowHandle;
            m_width = impl.m_width;
            m_height = impl.m_height;
            m_needCleanup = impl.m_needCleanup;
        }

        WindowImpl::WindowImpl(const char* windowName, int width, int height)
            : m_width(width), m_height(height), m_needCleanup{ true }
        {
            createWindow(width, height);
        }

        void WindowImpl::createWindow(int width, int height)
        {
            /*static TCHAR szAppName[] = TEXT("Darkness");
            WNDCLASS     wndclass;
            m_hInstance = GetModuleHandle(NULL);

            wndclass.style = CS_HREDRAW | CS_VREDRAW;
            wndclass.lpfnWndProc = WndProc;
            wndclass.cbClsExtra = 0;
            wndclass.cbWndExtra = 0;
            wndclass.hInstance = m_hInstance;
            wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
            wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wndclass.lpszMenuName = NULL;
            wndclass.lpszClassName = szAppName;

            if (!RegisterClass(&wndclass))
            {
                MessageBox(NULL, TEXT("This program requires Windows NT!"),
                    szAppName, MB_ICONERROR);
            }

            m_windowHandle = CreateWindow(szAppName, szAppName,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, m_hInstance, NULL);

            ShowWindow(m_windowHandle, SW_SHOW);
            SetWindowPos(m_windowHandle, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            UpdateWindow(m_windowHandle);*/
        }

        WindowImpl::~WindowImpl()
        {
            if (m_needCleanup)
            {
                //DestroyWindow(m_windowHandle);
                //m_windowHandle = NULL;

                //UnregisterClassA("Darkness", m_hInstance);
                //m_hInstance = NULL;
            }
        }

        const void* WindowImpl::handle() const
        {
            return m_windowHandle;
        }

        /*LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            }

            return DefWindowProc(hwnd, message, wParam, lParam);
        }*/
        
        void WindowImpl::setResizeCallback(ResizeCallback onResize)
        {
            //m_onResize = onResize;
        }
        
        bool WindowImpl::processMessages() const
        {
            /*MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    return false;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }*/
            return true;
        }
        
        void WindowImpl::resize(int width, int height)
        {
        }

        int WindowImpl::width() const
        {
            return m_width;
        }

        int WindowImpl::height() const
        {
            return m_height;
        }
    }
}
