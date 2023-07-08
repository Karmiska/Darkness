#pragma once

#include "containers/string.h"
#include "containers/memory.h"
#include <functional>
#include <thread>

namespace engine
{
    using OnProcessMessage = std::function<void(const engine::string&)>;
    class Process
    {
    public:
        Process(
            const engine::string& executable, 
            const engine::string& arguments,
            const engine::string& workingDirectory,
            OnProcessMessage onMessage
            );
        ~Process();
    private:
        engine::string m_executable;
        engine::string m_arguments;
        engine::string m_workingDirectory;
        OnProcessMessage m_onMessage;
        engine::unique_ptr<std::thread> m_work;
        void run();
    };
}
