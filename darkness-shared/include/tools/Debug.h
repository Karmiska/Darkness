#pragma once

#include "containers/string.h"
#include <functional>

using CustomDebugMessageHandler = std::function<void(const engine::string&)>;
extern CustomDebugMessageHandler customDebugMessageHandler;

void Debug(const char* location, const char* msg, ...);
void DebugInfo(const char* location, const char* msg, ...);
void DebugWarning(const char* location, const char* msg, ...);
void DebugError(const char* location, const char* msg, ...);

void DebugPure(const char* location, const char* msg, ...);
void DebugPureInfo(const char* location, const char* msg, ...);
void DebugPureWarning(const char* location, const char* msg, ...);
void DebugPureError(const char* location, const char* msg, ...);

void DebugRaw(const char*, const char* msg, ...);
void DebugRawInfo(const char*, const char* msg, ...);
void DebugRawWarning(const char*, const char* msg, ...);
void DebugRawError(const char*, const char* msg, ...);

void DebugAssert(const char* condition, const char* location);
void DebugAssert(const char* condition, const char* location, const char* msg, ...);

#define DBGSTRINGIFY(x) #x
#define DBGTOSTRING(x) DBGSTRINGIFY(x)
#define DBGLOC __FILE__ "(" DBGTOSTRING(__LINE__) "): "

#ifndef RETAIL
#define ASSERT(condition, ...) \
    (void)((condition) || (DebugAssert(#condition, DBGLOC, __VA_ARGS__),0))
#else
#define ASSERT(condition, ...)
#endif


#define LOG(...) Debug(DBGLOC, __VA_ARGS__)
#define LOG_INFO(...) DebugInfo(DBGLOC, __VA_ARGS__)
#define LOG_WARNING(...) DebugWarning(DBGLOC, __VA_ARGS__)
#define LOG_ERROR(...) DebugError(DBGLOC, __VA_ARGS__)

#define LOG_PURE(...) DebugPure(DBGLOC, __VA_ARGS__)
#define LOG_PURE_INFO(...) DebugPureInfo(DBGLOC, __VA_ARGS__)
#define LOG_PURE_WARNING(...) DebugPureWarning(DBGLOC, __VA_ARGS__)
#define LOG_PURE_ERROR(...) DebugPureError(DBGLOC, __VA_ARGS__)

#define LOG_RAW(...) DebugRaw("", __VA_ARGS__)
#define LOG_RAW_INFO(...) DebugRawInfo(DBGLOC, __VA_ARGS__)
#define LOG_RAW_WARNING(...) DebugRawWarning(DBGLOC, __VA_ARGS__)
#define LOG_RAW_ERROR(...) DebugRawError(DBGLOC, __VA_ARGS__)
