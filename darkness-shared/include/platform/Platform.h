#pragma once

#ifdef _WIN32
#pragma warning(push, 0) 
#include <WinSock2.h>
#ifndef _DURANGO
#include <Windows.h>
#else
//#include <Unknwnbase.h>
#endif
#pragma warning(pop)
namespace platform
{
#ifndef _DURANGO
    typedef HWND WindowHandle;
#else
	typedef void* WindowHandle;
#endif
}
#endif

#ifdef __APPLE__
namespace platform
{
    typedef void* WindowHandle;
}
#endif
