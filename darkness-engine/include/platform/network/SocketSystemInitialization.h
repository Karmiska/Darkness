#pragma once

namespace platform
{
    class SocketSystemInitialization
    {
    public:
        static SocketSystemInitialization& instance();

        void initialize();

        ~SocketSystemInitialization();
    private:
        SocketSystemInitialization();
        bool m_initialized;
    };
}
