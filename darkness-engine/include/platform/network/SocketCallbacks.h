#pragma once

#include <functional>
#include "containers/memory.h"

namespace platform
{
    class Socket;
    using OnConnect = std::function<void(engine::shared_ptr<Socket>)>;
    using OnDisconnect = std::function<void(engine::shared_ptr<Socket>)>;
    using OnData = std::function<void(engine::shared_ptr<Socket>, char*, size_t)>;
}
