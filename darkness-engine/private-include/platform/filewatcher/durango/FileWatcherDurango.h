#pragma once

#include "containers/string.h"
#include <functional>
#include "containers/vector.h"

namespace platform
{
	class WatchHandle;

	class FileWatcherImpl
	{
	public:
		WatchHandle addWatch(
			const engine::string& filePath,
			std::function<engine::string(const engine::string&)> onFileChange);

		void addNewChange(const engine::string& filename);

		bool hasChanges();
		bool processChanges();
		engine::vector<engine::string>& lastMessages();

	private:
		engine::vector<engine::string> m_messages;
	};
}
