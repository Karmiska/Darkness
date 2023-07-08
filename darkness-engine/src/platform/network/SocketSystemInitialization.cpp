#include "platform/network/SocketSystemInitialization.h"
#include "tools/Debug.h"

#ifdef _WIN32
#include <winsock.h>
#else
#ifdef OSX
#define Polygon CarbonPolygon
#include <Carbon/Carbon.h>
#undef Polygon
#endif

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace platform
{
    SocketSystemInitialization::SocketSystemInitialization()
        : m_initialized{ false }
    {};

    SocketSystemInitialization& SocketSystemInitialization::instance()
    {
        static SocketSystemInitialization instance;
        return instance;
    }

    void SocketSystemInitialization::initialize()
    {
        if (!m_initialized)
        {
#ifdef _WIN32
            WORD requestedVersion = MAKEWORD(2, 2);
            WSADATA wsaData;
            auto res = WSAStartup(requestedVersion, &wsaData);
            ASSERT(res == 0, "Could not initialize Networking");
#endif
            m_initialized = true;
        }
    }
    SocketSystemInitialization::~SocketSystemInitialization()
    {
        if (m_initialized)
        {
            WSACleanup();
        }
    }
}
