#include "engine/filesystem/VirtualFilesystem.h"
#include "platform/File.h"
#include "tools/Debug.h"

namespace engine
{
	engine::string resolvePath(const engine::string& filePath)
	{
#ifndef _DURANGO
		return filePath;
#endif
		if (filePath.find("data/shaders") != engine::string::npos)
		{
			auto cutStr = engine::string("D:/work/darkness/darkness-engine/data/");
			return "G:" + filePath.substr(cutStr.length(), filePath.length() - cutStr.length());
		}
		else if (filePath.find("DarknessDemo") != engine::string::npos)
		{
			auto cutStr = engine::string("C:/Users/aleksi.jokinen/Documents/");
			return "G:" + filePath.substr(cutStr.length(), filePath.length() - cutStr.length());
		}
		else
		{
			LOG_ERROR("Path not found: %s", filePath.c_str());
		}
		return filePath;
	}
}
