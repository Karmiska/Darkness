#include "platform/FileWatcher.h"

#ifdef _WIN32
#ifndef _DURANGO
#include "platform/filewatcher/windows/FileWatcherWin.h"
#else
#include "platform/filewatcher/durango/FileWatcherDurango.h"
#endif
#endif

namespace platform
{
    FileWatcher::FileWatcher()
        : m_impl{ engine::make_shared<FileWatcherImpl>() }
    {
    }

    WatchHandle FileWatcher::addWatch(
        const engine::string& filePath,
        std::function<engine::string(const engine::string&)> onFileChange)
    {
        return m_impl->addWatch(filePath, onFileChange);
    }

    bool FileWatcher::hasChanges()
    {
        return m_impl->hasChanges();
    }

    bool FileWatcher::processChanges()
    {
        return m_impl->processChanges();
    }

    engine::vector<engine::string>& FileWatcher::lastMessages()
    {
        return m_impl->lastMessages();
    }
}
