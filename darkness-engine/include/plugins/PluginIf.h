#pragma once

#include "containers/string.h"

namespace engine
{
    class PluginIf
    {
    public:
        virtual ~PluginIf() {};
        virtual engine::string name() const = 0;
    };

    class Plugin : public PluginIf
    {
        engine::string name() const override;
    };
}

extern "C"
{
    engine::PluginIf* createInstance();
    void destroyInstance(engine::PluginIf*);
}
