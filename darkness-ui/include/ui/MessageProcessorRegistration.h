#pragma once

#include "containers/memory.h"
#include "containers/vector.h"

namespace ui
{
	class Frame;
	class MessageProcessorRegistration
	{
	public:
		MessageProcessorRegistration(Frame* frame);
		MessageProcessorRegistration(const MessageProcessorRegistration&) = delete;
		MessageProcessorRegistration& operator=(const MessageProcessorRegistration&) = delete;
		MessageProcessorRegistration(MessageProcessorRegistration&& registration);
		MessageProcessorRegistration& operator=(MessageProcessorRegistration&& registration);
		~MessageProcessorRegistration();
	private:
		Frame* m_frame;
	};
}
