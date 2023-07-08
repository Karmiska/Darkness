#pragma once

#include "platform/window/Window.h"
#include "platform/Platform.h"

namespace platform
{
    namespace implementation
    {
        class WindowImpl
        {
        public:
            WindowImpl(void* handle, int width, int height);
            WindowImpl(const char* windowName, int width, int height);
            WindowImpl(const WindowImpl& impl);
            ~WindowImpl();

            void setResizeCallback(ResizeCallback onResize);
            
            bool processMessages() const;
            int width() const;
            int height() const;
            void resize(int width, int height);
            const void* handle() const;

        private:
            bool m_needCleanup;
            int m_width, m_height;
            void* m_windowHandle;
            void* m_hInstance;
            void createWindow(int width, int height);
        };
    }
}
