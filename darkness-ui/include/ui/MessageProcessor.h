#pragma once

#include "containers/memory.h"
#include "containers/vector.h"
#include "ui/Frame.h"

namespace ui
{
	class MessageProcessor
	{
	public:
		void registerRootFrame(Frame* frame);
		void unregisterRootFrame(Frame* frame);
		bool processMessages();

	private:
		engine::vector<Frame*> m_rootFrames;
		engine::vector<Frame*> m_removedFrames;

	public:
		static MessageProcessor& instance()
		{
			static MessageProcessor m_messageProcessor;
			return m_messageProcessor;
		};

	private:
		MessageProcessor() {};
	};
}
