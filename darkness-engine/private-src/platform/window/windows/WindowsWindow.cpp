#ifndef _DURANGO

#include "platform/window/windows/WindowsWindow.h"
#include "tools/Debug.h"
#ifdef _UNICODE
#include <cstdlib>
#endif
#include <windowsx.h>

namespace platform
{
    namespace implementation
    {
        LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        WindowImpl::WindowImpl(HWND handle, int width, int height, bool createOwnWindow)
            : m_windowHandle{ handle }
			, m_quitSignaled{ false }
            , m_width(width)
            , m_height(height)
            , m_mouseCursor{ MouseCursor::Arrow }
            , m_onResize{ nullptr }
			, m_mainWindow{ false }
			, m_windowName{""}
            , m_createdFromHandle{ true }
            , m_isMaximized{ false }
            , m_isMinimized{ false }
            , m_isResizing{ false }
			, m_disableNotification{ false }
            , m_lastMouseX{ 0 }
            , m_lastMouseY{ 0 }
        {
			if (createOwnWindow)
			{
				m_parentWindowHandle = handle;
				createWindow(handle, "SubWindow", width, height);
			}
        }


        void WindowImpl::createCursors()
        {
            m_cursors[0] = LoadCursor(nullptr, IDC_ARROW);
            m_cursors[1] = LoadCursor(nullptr, IDC_IBEAM);
            m_cursors[2] = LoadCursor(nullptr, IDC_WAIT);
            m_cursors[3] = LoadCursor(nullptr, IDC_CROSS);
            m_cursors[4] = LoadCursor(nullptr, IDC_UPARROW);
            m_cursors[5] = LoadCursor(nullptr, IDC_SIZENWSE);
            m_cursors[6] = LoadCursor(nullptr, IDC_SIZENESW);
            m_cursors[7] = LoadCursor(nullptr, IDC_SIZEWE);
            m_cursors[8] = LoadCursor(nullptr, IDC_SIZENS);
            m_cursors[9] = LoadCursor(nullptr, IDC_SIZEALL);
            m_cursors[10] = LoadCursor(nullptr, IDC_NO);
        }
        /*WindowImpl::WindowImpl(const WindowImpl& impl)
        {
            ASSERT(!impl.m_needCleanup);
            m_windowHandle = impl.m_windowHandle;
            m_width = impl.m_width;
            m_height = impl.m_height;
            m_needCleanup = impl.m_needCleanup;
			m_quitSignaled = impl.m_quitSignaled;
            m_createdFromHandle = impl.m_createdFromHandle;

			m_onMouseMove = impl.m_onMouseMove;
			m_onMouseDown = impl.m_onMouseDown;
			m_onMouseUp = impl.m_onMouseUp;
			m_onMouseDoubleClick = impl.m_onMouseDoubleClick;
			m_onKeyDown = impl.m_onKeyDown;
			m_onKeyUp = impl.m_onKeyUp;

			m_isMaximized = impl.m_isMaximized;
			m_isMinimized = impl.m_isMinimized;
			m_isResizing = impl.m_isResizing;
        }*/

        WindowImpl::WindowImpl(const char* windowName, int width, int height)
            : m_width(width)
            , m_height(height)
            , m_mouseCursor{ MouseCursor::Arrow }
			, m_quitSignaled{ false }
            , m_createdFromHandle{ false }
			, m_windowName{""}
			, m_disableNotification{ false }
            , m_lastMouseX{ 0 }
            , m_lastMouseY{ 0 }
        {
            createWindow(NULL, windowName, width, height);
        }

#include <strsafe.h>

		class WindowRegistrations
		{
		public:
			static WindowRegistrations& instance()
			{
				static WindowRegistrations registrations;
				return registrations;
			}

			int getNewId()
			{
				return m_currentId++;
			};
		private:
			WindowRegistrations() : m_currentId{ 0 } {};
			int m_currentId;
		};


        void WindowImpl::createWindow(HWND parentWindow, const char* windowName, int width, int height)
        {
			engine::string windowClassName = windowName;
			auto newWindowId = WindowRegistrations::instance().getNewId();
			windowClassName += "_" + std::to_string(newWindowId);
            #ifdef _UNICODE
            static TCHAR szAppName[1024] = {};
            size_t numCharacters;
            mbstowcs_s(&numCharacters, szAppName, windowClassName.c_str(), 1024);
            #else
            TCHAR szAppName[1024] = {};
            strncpy_s(szAppName, windowClassName.c_str(), windowClassName.length());
            #endif
            WNDCLASS     wndclass;
            m_hInstance = GetModuleHandle(NULL);

			m_windowName = windowClassName;
			m_mainWindow = newWindowId == 0;

            wndclass.style = CS_HREDRAW | CS_VREDRAW;
            wndclass.lpfnWndProc = WndProc;
            wndclass.cbClsExtra = 0;
            wndclass.cbWndExtra = 0;
            wndclass.hInstance = m_hInstance;
            wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wndclass.hCursor = NULL;// LoadCursor(NULL, IDC_ARROW);
            wndclass.hbrBackground = parentWindow ? (HBRUSH)::GetStockObject(DKGRAY_BRUSH) : (HBRUSH)::GetStockObject(WHITE_BRUSH);
            wndclass.lpszMenuName = NULL;
            wndclass.lpszClassName = szAppName;

            if (!RegisterClass(&wndclass))
            {
				LPVOID lpMsgBuf;
				LPVOID lpDisplayBuf;
				DWORD dw = GetLastError();

				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0, NULL);

				lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
					(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
				StringCchPrintf((LPTSTR)lpDisplayBuf,
					LocalSize(lpDisplayBuf) / sizeof(TCHAR),
					TEXT("failed with error %d: %s"), dw, lpMsgBuf);
				MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
            }

            m_windowHandle = CreateWindowEx(
                0,
                szAppName,
                szAppName,
                parentWindow ? WS_CHILDWINDOW : WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
				parentWindow,
                NULL,
                m_hInstance,
                this);

            /*m_windowHandle = CreateWindow(szAppName, szAppName,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, m_hInstance, NULL);*/

            createCursors();

            RECT windowRect{ 0, 0, width, height };
            AdjustWindowRectEx(&windowRect, parentWindow ? WS_CHILDWINDOW : WS_OVERLAPPEDWINDOW, false, 0);

            ShowWindow(m_windowHandle, SW_SHOW);
            SetWindowPos(m_windowHandle, 0, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            UpdateWindow(m_windowHandle);
            refreshSize();
        }

		void WindowImpl::position(int x, int y)
		{
			RECT windowRect{ x, y, x + m_width, y + m_height };
			m_disableNotification = true;
			AdjustWindowRectEx(&windowRect, m_parentWindowHandle ? WS_CHILDWINDOW : WS_OVERLAPPEDWINDOW, false, 0);
			
			SetWindowPos(
				m_windowHandle,
				0, 
				windowRect.left, 
				windowRect.top, 
				m_width,
				m_height,
				0);// SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			//UpdateWindow(m_windowHandle);

			m_disableNotification = false;
		}

        WindowImpl::~WindowImpl()
        {
            DestroyWindow(m_windowHandle);
            m_windowHandle = NULL;

            auto res = UnregisterClassA(m_windowName.c_str(), m_hInstance);
            ASSERT(res == TRUE, "Failed to unregister a window class");
            m_hInstance = NULL;
        }

        const HWND WindowImpl::handle() const
        {
            return m_windowHandle;
        }

        void WindowImpl::setHandle(HWND handle)
        {
            m_windowHandle = handle;
        }

        LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            WindowImpl* windowInstance = nullptr;
            if (message == WM_NCCREATE)
            {
                LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
                windowInstance = reinterpret_cast<WindowImpl*>(lpcs->lpCreateParams);
                windowInstance->setHandle(hwnd);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(windowInstance));
            }
            else
            {
                windowInstance = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            }

            if (windowInstance)
            {
                return windowInstance->handleMsg(message, wParam, lParam);
            }
            else
            {
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }

        void WindowImpl::refreshSize()
        {
            RECT clientRect = {};
            GetClientRect(m_windowHandle, &clientRect);
            m_width = clientRect.right - clientRect.left;
            m_height = clientRect.bottom - clientRect.top;
        }

        void WindowImpl::conditionalRefresSize(int width, int height)
        {
            bool notify = (width != m_width) || (height != m_height);
            m_width = width;
            m_height = height;

            if (notify && m_onResize && !m_disableNotification)
            {
                m_onResize(m_width, m_height);
            }
        }

        LRESULT WindowImpl::handleMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            if (handleMouseMsg(uMsg, wParam, lParam))
                return DefWindowProc(m_windowHandle, uMsg, wParam, lParam);

            if (handleKeyboardMsg(uMsg, wParam, lParam))
                return DefWindowProc(m_windowHandle, uMsg, wParam, lParam);

            if(handleSizeMsg(uMsg, wParam, lParam))
                return DefWindowProc(m_windowHandle, uMsg, wParam, lParam);

            if(handleCursorMsg(uMsg, wParam, lParam))
                return TRUE;

            switch (uMsg)
            {
                case WM_DESTROY:
                {
					if(m_mainWindow)
						PostQuitMessage(0);
					m_quitSignaled = true;
                    return 0;
                }
                case WM_MENUCHAR:
                {
                    return MAKELRESULT(0, MNC_CLOSE);
                }
            }
            return DefWindowProc(m_windowHandle, uMsg, wParam, lParam);
        }

        bool WindowImpl::handleSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            switch (uMsg)
            {
                case WM_ENTERSIZEMOVE:
                {
                    m_isResizing = true;
                    return true;
                }
                case WM_MOVING:
                {
                    return true;
                }
				case WM_SIZING:
				{
					RECT clientRect = {};
					GetClientRect(m_windowHandle, &clientRect);
					auto width = clientRect.right - clientRect.left;
					auto height = clientRect.bottom - clientRect.top;

					conditionalRefresSize(width, height);

					return true;
				}
                case WM_EXITSIZEMOVE:
                {
                    m_isResizing = false;
                    
                    RECT clientRect = {};
                    GetClientRect(m_windowHandle, &clientRect);
                    auto width = clientRect.right - clientRect.left;
                    auto height = clientRect.bottom - clientRect.top;

                    conditionalRefresSize(width, height);
                    return true;
                }
                case WM_SIZE:
                {
                    if (wParam == SIZE_MINIMIZED)
                    {
                        m_isMinimized = true;
                        m_isMaximized = false;
                    }
                    else if (wParam == SIZE_MAXIMIZED)
                    {
                        m_isMinimized = false;
                        m_isMaximized = true;
                        conditionalRefresSize(LOWORD(lParam), HIWORD(lParam));
                    }
                    else if (wParam == SIZE_RESTORED)
                    {
                        if (m_isMinimized)
                        {
                            m_isMinimized = false;
                        }
                        else if (m_isMaximized)
                        {
                            m_isMaximized = false;
                        }
                        else if (m_isResizing)
                        {
                            // nop
                        }
                        else
                        {
                        }
                        conditionalRefresSize(LOWORD(lParam), HIWORD(lParam));
                    }
                    return true;
                }
            }
            return false;
        }

        bool WindowImpl::handleKeyboardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            switch (uMsg)
            {
                case WM_KEYDOWN:
                {
                    auto key = engineKeyFromWindows(wParam, lParam);
                    if (m_onKeyDown)
                        m_onKeyDown(key, modifierState());
                    return true;
                }
                case WM_KEYUP:
                {
                    auto key = engineKeyFromWindows(wParam, lParam);
                    if (m_onKeyUp)
                        m_onKeyUp(key, modifierState());
                    return true;
                }
                case WM_SYSKEYDOWN:
                {
                    auto key = engineKeyFromWindows(wParam, lParam);
                    if (m_onKeyDown)
                        m_onKeyDown(key, modifierState());
                    return true;
                }
                case WM_SYSKEYUP:
                {
                    auto key = engineKeyFromWindows(wParam, lParam);
                    if (m_onKeyUp)
                        m_onKeyUp(key, modifierState());
                    return true;
                }
            }
            return false;
        }

        engine::Key WindowImpl::engineKeyFromWindows(WPARAM wParam, LPARAM /*lParam*/) const
        {
            switch(wParam)
            {
                case VK_LBUTTON:                return engine::Key::Unknown;
                case VK_RBUTTON:                return engine::Key::Unknown;
                case VK_CANCEL:                 return engine::Key::Unknown;
                case VK_MBUTTON:                return engine::Key::Unknown;
                case VK_XBUTTON1:               return engine::Key::Unknown;
                case VK_XBUTTON2:               return engine::Key::Unknown;
                case VK_BACK:                   return engine::Key::Backspace;
                case VK_TAB:                    return engine::Key::Tab;
                case VK_CLEAR:                  return engine::Key::Unknown;
                case VK_RETURN:                 return engine::Key::Enter;
                case VK_SHIFT:                  return engine::Key::Unknown;
                case VK_CONTROL:                return engine::Key::Unknown;
                case VK_MENU:                   return engine::Key::Unknown;
                case VK_PAUSE:                  return engine::Key::Pause;
                case VK_CAPITAL:                return engine::Key::Unknown;
                case VK_KANA:                   return engine::Key::Unknown;
                case VK_JUNJA:                  return engine::Key::Unknown;
                case VK_FINAL:                  return engine::Key::Unknown;
                case VK_HANJA:                  return engine::Key::Unknown;
                case VK_ESCAPE:                 return engine::Key::Escape;
                case VK_CONVERT:                return engine::Key::Unknown;
                case VK_NONCONVERT:             return engine::Key::Unknown;
                case VK_ACCEPT:                 return engine::Key::Unknown;
                case VK_MODECHANGE:             return engine::Key::Unknown;
                case VK_SPACE:                  return engine::Key::Space;
                case VK_PRIOR:                  return engine::Key::PageUp;
                case VK_NEXT:                   return engine::Key::PageDown;
                case VK_END:                    return engine::Key::End;
                case VK_HOME:                   return engine::Key::Home;
                case VK_LEFT:                   return engine::Key::ArrowLeft;
                case VK_UP:                     return engine::Key::ArrowUp;
                case VK_RIGHT:                  return engine::Key::ArrowRight;
                case VK_DOWN:                   return engine::Key::ArrowDown;
                case VK_SELECT:                 return engine::Key::Unknown;
                case VK_PRINT:                  return engine::Key::PrintScreen;
                case VK_EXECUTE:                return engine::Key::Unknown;
                case VK_SNAPSHOT:               return engine::Key::Unknown;
                case VK_INSERT:                 return engine::Key::Insert;
                case VK_DELETE:                 return engine::Key::Delete;
                case VK_HELP:                   return engine::Key::Unknown;
                case 0x30:                      return engine::Key::Num0;
                case 0x31:                      return engine::Key::Num1;
                case 0x32:                      return engine::Key::Num2;
                case 0x33:                      return engine::Key::Num3;
                case 0x34:                      return engine::Key::Num4;
                case 0x35:                      return engine::Key::Num5;
                case 0x36:                      return engine::Key::Num6;
                case 0x37:                      return engine::Key::Num7;
                case 0x38:                      return engine::Key::Num8;
                case 0x39:                      return engine::Key::Num9;
                case 0x41:                      return engine::Key::A;
                case 0x42:                      return engine::Key::B;
                case 0x43:                      return engine::Key::C;
                case 0x44:                      return engine::Key::D;
                case 0x45:                      return engine::Key::E;
                case 0x46:                      return engine::Key::F;
                case 0x47:                      return engine::Key::G;
                case 0x48:                      return engine::Key::H;
                case 0x49:                      return engine::Key::I;
                case 0x4A:                      return engine::Key::J;
                case 0x4B:                      return engine::Key::K;
                case 0x4C:                      return engine::Key::L;
                case 0x4D:                      return engine::Key::M;
                case 0x4E:                      return engine::Key::N;
                case 0x4F:                      return engine::Key::O;
                case 0x50:                      return engine::Key::P;
                case 0x51:                      return engine::Key::Q;
                case 0x52:                      return engine::Key::R;
                case 0x53:                      return engine::Key::S;
                case 0x54:                      return engine::Key::T;
                case 0x55:                      return engine::Key::U;
                case 0x56:                      return engine::Key::V;
                case 0x57:                      return engine::Key::W;
                case 0x58:                      return engine::Key::X;
                case 0x59:                      return engine::Key::Y;
                case 0x5A:                      return engine::Key::Z;
                case VK_LWIN:                   return engine::Key::Unknown;
                case VK_RWIN:                   return engine::Key::Unknown;
                case VK_APPS:                   return engine::Key::Unknown;
                case VK_SLEEP:                  return engine::Key::Unknown;
                case VK_NUMPAD0:                return engine::Key::Num0;
                case VK_NUMPAD1:                return engine::Key::Num1;
                case VK_NUMPAD2:                return engine::Key::Num2;
                case VK_NUMPAD3:                return engine::Key::Num3;
                case VK_NUMPAD4:                return engine::Key::Num4;
                case VK_NUMPAD5:                return engine::Key::Num5;
                case VK_NUMPAD6:                return engine::Key::Num6;
                case VK_NUMPAD7:                return engine::Key::Num7;
                case VK_NUMPAD8:                return engine::Key::Num8;
                case VK_NUMPAD9:                return engine::Key::Num9;
                case VK_MULTIPLY:               return engine::Key::Unknown;
                case VK_ADD:                    return engine::Key::Unknown;
                case VK_SEPARATOR:              return engine::Key::Unknown;
                case VK_SUBTRACT:               return engine::Key::Unknown;
                case VK_DECIMAL:                return engine::Key::Unknown;
                case VK_DIVIDE:                 return engine::Key::Unknown;
                case VK_F1:                     return engine::Key::F1;
                case VK_F2:                     return engine::Key::F2;
                case VK_F3:                     return engine::Key::F3;
                case VK_F4:                     return engine::Key::F4;
                case VK_F5:                     return engine::Key::F5;
                case VK_F6:                     return engine::Key::F6;
                case VK_F7:                     return engine::Key::F7;
                case VK_F8:                     return engine::Key::F8;
                case VK_F9:                     return engine::Key::F9;
                case VK_F10:                    return engine::Key::F10;
                case VK_F11:                    return engine::Key::F11;
                case VK_F12:                    return engine::Key::F12;
                case VK_F13:                    return engine::Key::Unknown;
                case VK_F14:                    return engine::Key::Unknown;
                case VK_F15:                    return engine::Key::Unknown;
                case VK_F16:                    return engine::Key::Unknown;
                case VK_F17:                    return engine::Key::Unknown;
                case VK_F18:                    return engine::Key::Unknown;
                case VK_F19:                    return engine::Key::Unknown;
                case VK_F20:                    return engine::Key::Unknown;
                case VK_F21:                    return engine::Key::Unknown;
                case VK_F22:                    return engine::Key::Unknown;
                case VK_F23:                    return engine::Key::Unknown;
                case VK_F24:                    return engine::Key::Unknown;
                case VK_NUMLOCK:                return engine::Key::Unknown;
                case VK_SCROLL:                 return engine::Key::ScrollLock;
                case VK_LSHIFT:                 return engine::Key::Unknown;
                case VK_RSHIFT:                 return engine::Key::Unknown;
                case VK_LCONTROL:               return engine::Key::Unknown;
                case VK_RCONTROL:               return engine::Key::Unknown;
                case VK_LMENU:                  return engine::Key::Unknown;
                case VK_RMENU:                  return engine::Key::Unknown;
                case VK_BROWSER_BACK:           return engine::Key::Unknown;
                case VK_BROWSER_FORWARD:        return engine::Key::Unknown;
                case VK_BROWSER_REFRESH:        return engine::Key::Unknown;
                case VK_BROWSER_STOP:           return engine::Key::Unknown;
                case VK_BROWSER_SEARCH:         return engine::Key::Unknown;
                case VK_BROWSER_FAVORITES:      return engine::Key::Unknown;
                case VK_BROWSER_HOME:           return engine::Key::Unknown;
                case VK_VOLUME_MUTE:            return engine::Key::Unknown;
                case VK_VOLUME_DOWN:            return engine::Key::Unknown;
                case VK_VOLUME_UP:              return engine::Key::Unknown;
                case VK_MEDIA_NEXT_TRACK:       return engine::Key::Unknown;
                case VK_MEDIA_PREV_TRACK:       return engine::Key::Unknown;
                case VK_MEDIA_STOP:             return engine::Key::Unknown;
                case VK_MEDIA_PLAY_PAUSE:       return engine::Key::Unknown;
                case VK_LAUNCH_MAIL:            return engine::Key::Unknown;
                case VK_LAUNCH_MEDIA_SELECT:    return engine::Key::Unknown;
                case VK_LAUNCH_APP1:            return engine::Key::Unknown;
                case VK_LAUNCH_APP2:            return engine::Key::Unknown;
                case VK_OEM_1:                  return engine::Key::Unknown;
                case VK_OEM_PLUS:               return engine::Key::Unknown;
                case VK_OEM_COMMA:              return engine::Key::Comma;
                case VK_OEM_MINUS:              return engine::Key::Unknown;
                case VK_OEM_PERIOD:             return engine::Key::Period;
                case VK_OEM_2:                  return engine::Key::Unknown;
                case VK_OEM_3:                  return engine::Key::Unknown;
                case VK_OEM_4:                  return engine::Key::Unknown;
                case VK_OEM_5:                  return engine::Key::Unknown;
                case VK_OEM_6:                  return engine::Key::Unknown;
                case VK_OEM_7:                  return engine::Key::Unknown;
                case VK_OEM_8:                  return engine::Key::Unknown;
                case VK_OEM_102:                return engine::Key::Unknown;
                case VK_PROCESSKEY:             return engine::Key::Unknown;
                case VK_PACKET:                 return engine::Key::Unknown;
                case VK_ATTN:                   return engine::Key::Unknown;
                case VK_CRSEL:                  return engine::Key::Unknown;
                case VK_EXSEL:                  return engine::Key::Unknown;
                case VK_EREOF:                  return engine::Key::Unknown;
                case VK_PLAY:                   return engine::Key::Unknown;
                case VK_ZOOM:                   return engine::Key::Unknown;
                case VK_NONAME:                 return engine::Key::Unknown;
                case VK_PA1:                    return engine::Key::Unknown;
                case VK_OEM_CLEAR:              return engine::Key::Unknown;
            }
			return engine::Key::Unknown;
        }

        engine::ModifierState WindowImpl::modifierState() const
        {
            engine::ModifierState modState;
            modState[engine::KeyModifier::ShiftLeft] = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
            modState[engine::KeyModifier::ShiftRight] = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
            modState[engine::KeyModifier::AltLeft] = GetAsyncKeyState(VK_MENU) & 0x8000;
            modState[engine::KeyModifier::AltRight] = GetAsyncKeyState(VK_MENU) & 0x8000;
            modState[engine::KeyModifier::CtrlLeft] = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
            modState[engine::KeyModifier::CtrlRight] = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
            return modState;
        }

        bool WindowImpl::handleMouseMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            switch (uMsg)
            {
                case WM_MOUSEMOVE:
                {
                    if (m_onMouseMove)
                        m_onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    m_lastMouseX = GET_X_LPARAM(lParam);
                    m_lastMouseY = GET_Y_LPARAM(lParam);
                    return true;
                }

                case WM_LBUTTONDOWN:
                {
                    if (m_onMouseDown)
                        m_onMouseDown(
                            engine::MouseButton::Left,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_MBUTTONDOWN:
                {
                    if (m_onMouseDown)
                        m_onMouseDown(
                            engine::MouseButton::Center,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_RBUTTONDOWN:
                {
                    if (m_onMouseDown)
                        m_onMouseDown(
                            engine::MouseButton::Right,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_LBUTTONUP:
                {
                    if (m_onMouseUp)
                        m_onMouseUp(
                            engine::MouseButton::Left,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_MBUTTONUP:
                {
                    if (m_onMouseUp)
                        m_onMouseUp(
                            engine::MouseButton::Center,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_RBUTTONUP:
                {
                    if (m_onMouseUp)
                        m_onMouseUp(
                            engine::MouseButton::Right,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_LBUTTONDBLCLK:
                {
                    if (m_onMouseDoubleClick)
                        m_onMouseDoubleClick(
                            engine::MouseButton::Left,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_MBUTTONDBLCLK:
                {
                    if (m_onMouseDoubleClick)
                        m_onMouseDoubleClick(
                            engine::MouseButton::Center,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
                case WM_RBUTTONDBLCLK:
                {
                    if (m_onMouseDoubleClick)
                        m_onMouseDoubleClick(
                            engine::MouseButton::Right,
                            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    return true;
                }
				case WM_MOUSEWHEEL:
				{
                    // NOTE!! Mouse wheel event have incorrect mouse coordinates
                    // it returns the location in the display instead of the application window
                    // this is here hacked because of it!
					if (m_onMouseWheel)
                        m_onMouseWheel(m_lastMouseX, m_lastMouseY,
                            GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
						//m_onMouseWheel(GET_X_LPARAM(lParam), 
						//	GET_Y_LPARAM(lParam), 
						//	GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
					return true;
				}
            }
            return false;
        }

        bool WindowImpl::handleCursorMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
        {
            if (uMsg == WM_SETCURSOR)
            {
                /*if (GetCursor() != m_cursors[static_cast<int>(m_mouseCursor)])
                {
                    auto oldCursor = SetCursor(m_cursors[static_cast<int>(m_mouseCursor)]);
                    return true;
                }*/
            }
            return false;
        }

        bool WindowImpl::processMessages() const
        {
			if (m_quitSignaled)
				return false;

            MSG msg;
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                {
					m_quitSignaled = true;
                    return false;
                }
            }
            return true;
        }

        void WindowImpl::setResizeCallback(ResizeCallback onResize)
        {
            m_onResize = onResize;
        }

        void WindowImpl::setMouseCallbacks(
            MouseMoveCallback           onMouseMove,
            MouseDownCallback           onMouseDown,
            MouseUpCallback             onMouseUp,
            MouseDoubleClickCallback    onMouseDoubleClick,
			MouseWheelCallback			onMouseWheel)
        {
            m_onMouseMove = onMouseMove;
            m_onMouseDown = onMouseDown;
            m_onMouseUp = onMouseUp;
            m_onMouseDoubleClick = onMouseDoubleClick;
			m_onMouseWheel = onMouseWheel;
        }

        void WindowImpl::setKeyboardCallbacks(
            KeyDownCallback onKeyDown,
            KeyUpCallback   onKeyUp)
        {
            m_onKeyDown = onKeyDown;
            m_onKeyUp = onKeyUp;
        }

        int WindowImpl::width() const
        {
            return m_width;
        }

        int WindowImpl::height() const
        {
            return m_height;
        }

        void WindowImpl::resize(int width, int height)
        {
            if (m_width == width && m_height == height)
                return;
            m_width = width;
            m_height = height;
            //if (!m_createdFromHandle)
            {
                SetWindowPos(m_windowHandle, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }

		WindowHandle WindowImpl::native() const
		{
			return m_windowHandle;
		}

        MouseCursor WindowImpl::mouseCursor() const
        {
            return m_mouseCursor;
        }

        void WindowImpl::mouseCursor(MouseCursor cursor)
        {
            m_mouseCursor = cursor;
            /*auto oldCursor = */SetCursor(m_cursors[static_cast<int>(m_mouseCursor)]);
        }
    }
}

#endif
