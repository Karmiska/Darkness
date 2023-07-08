#include "platform/filewatcher/durango/FileWatcherDurango.h"
#include "platform/FileWatcher.h"

namespace platform
{
	WatchHandle FileWatcherImpl::addWatch(
		const engine::string& /*filePath*/,
		std::function<engine::string(const engine::string&)> /*onFileChange*/)
	{
		return {};
	}

	void FileWatcherImpl::addNewChange(const engine::string& /*filename*/)
	{}

	bool FileWatcherImpl::hasChanges()
	{
		return false;
	}

	bool FileWatcherImpl::processChanges()
	{
		return false;
	}

	engine::vector<engine::string>& FileWatcherImpl::lastMessages()
	{
		return m_messages;
	}
}