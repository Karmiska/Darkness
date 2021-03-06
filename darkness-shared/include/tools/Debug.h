#pragma once

#include <string>
#include <functional>

using CustomDebugMessageHandler = std::function<void(const std::string&)>;
extern CustomDebugMessageHandler customDebugMessageHandler;

void Debug(const char* location, const char* msg, ...);
void DebugInfo(const char* location, const char* msg, ...);
void DebugWarning(const char* location, const char* msg, ...);
void DebugError(const char* location, const char* msg, ...);

void DebugPure(const char* location, const char* msg, ...);
void DebugPureInfo(const char* location, const char* msg, ...);
void DebugPureWarning(const char* location, const char* msg, ...);
void DebugPureError(const char* location, const char* msg, ...);

void DebugAssert(const char* condition, const char* location);
void DebugAssert(const char* condition, const char* location, const char* msg, ...);

#define DBGSTRINGIFY(x) #x
#define DBGTOSTRING(x) DBGSTRINGIFY(x)
#define DBGLOC __FILE__ "(" DBGTOSTRING(__LINE__) "): "


#define ASSERT(condition, ...) \
    (void)((condition) || (DebugAssert(#condition, DBGLOC, __VA_ARGS__),0))


#define LOG(...) Debug(DBGLOC, __VA_ARGS__)
#define LOG_INFO(...) DebugInfo(DBGLOC, __VA_ARGS__)
#define LOG_WARNING(...) DebugWarning(DBGLOC, __VA_ARGS__)
#define LOG_ERROR(...) DebugError(DBGLOC, __VA_ARGS__)

#define LOG_PURE(...) DebugPure(DBGLOC, __VA_ARGS__)
#define LOG_PURE_INFO(...) DebugPureInfo(DBGLOC, __VA_ARGS__)
#define LOG_PURE_WARNING(...) DebugPureWarning(DBGLOC, __VA_ARGS__)
#define LOG_PURE_ERROR(...) DebugPureError(DBGLOC, __VA_ARGS__)
