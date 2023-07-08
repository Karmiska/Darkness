#pragma once

#include "engine/graphics/dx12/DX12Headers.h"
#include "tools/Debug.h"

namespace engine
{
    DXGI_FORMAT GetBaseFormat(DXGI_FORMAT defaultFormat);

    DXGI_FORMAT GetUAVFormat(DXGI_FORMAT defaultFormat);

    DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat);

    DXGI_FORMAT GetDepthFormat(DXGI_FORMAT defaultFormat);

    DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat);
}
