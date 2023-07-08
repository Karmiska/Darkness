#pragma once

#include "containers/vector.h"
#include "containers/string.h"
#include "engine/rendering/ModelCpu.h"

namespace resource_task
{
    class Simplygon
    {
    public:
        Simplygon();
        ~Simplygon();

        struct PackedGeometry
        {
            engine::vector<engine::ModelCpu> lods;
        };
        PackedGeometry generateLod(
            uint32_t lodCount,
            engine::ModelCpu& model
        );
    private:
        engine::string getLicenseText() const;
    };
}
