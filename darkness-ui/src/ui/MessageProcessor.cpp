#include "ui/MessageProcessor.h"
#include "platform/window/Window.h"
#include <algorithm>

namespace ui
{
	void MessageProcessor::registerRootFrame(Frame* frame)
	{
		m_rootFrames.emplace_back(frame);
	}

	void MessageProcessor::unregisterRootFrame(Frame* frame)
	{
		auto found = std::find(m_rootFrames.begin(), m_rootFrames.end(), frame);
		if (found != m_rootFrames.end())
			m_removedFrames.emplace_back(frame);
	}

	bool MessageProcessor::processMessages()
	{
		bool result = false;
		for (auto&& frame : m_rootFrames)
		{
			if (frame->window() && !frame->window()->processMessages())
			{
				frame->frameMessageLoopClose();
				return true;
			}
			result = true;
			if (m_removedFrames.size() > 0)
				break;
		}

		for(auto&& frame : m_removedFrames)
		{
			auto found = std::find(m_rootFrames.begin(), m_rootFrames.end(), frame);
			if (found != m_rootFrames.end())
				m_rootFrames.erase(found);
		}
		m_removedFrames.clear();

		return result;
	}
}
