#include "ui/MessageProcessorRegistration.h"
#include "ui/MessageProcessor.h"
#include "ui/Frame.h"

namespace ui
{
	MessageProcessorRegistration::MessageProcessorRegistration(Frame* frame)
		: m_frame{ frame }
	{
		if (m_frame)
			MessageProcessor::instance().registerRootFrame(m_frame);
	}

	MessageProcessorRegistration::MessageProcessorRegistration(MessageProcessorRegistration&& registration)
		: m_frame{ nullptr }
	{
		std::swap(m_frame, registration.m_frame);
	}

	MessageProcessorRegistration& MessageProcessorRegistration::operator=(MessageProcessorRegistration&& registration)
	{
		m_frame = nullptr;
		std::swap(m_frame, registration.m_frame);
		return *this;
	}

	MessageProcessorRegistration::~MessageProcessorRegistration()
	{
		if (m_frame)
			MessageProcessor::instance().unregisterRootFrame(m_frame);
	}

}