#pragma once

#include "engine/graphics/Pipeline.h"
#include "shaders/core/tools/MathOperation.h"
#include "shaders/core/tools/MathOperationConstant.h"
#include "shaders/core/tools/MathOperationSameOutput.h"

namespace engine
{
    class Device;
    class BufferSRV;
    class BufferUAV;
    class CommandList;

    enum class BufferMathOperation
    {
        Addition,
        Subtraction,
        Division,
        Multiplication,
        Set
    };

    class BufferMath
    {
    public:
        BufferMath(Device& device);

        void perform(
            CommandList& cmd,
            const BufferSRV srcA,
            const BufferSRV srcB,
            BufferUAV output,
            BufferMathOperation op,
            uint32_t count,
            uint32_t srcAOffset,
            uint32_t srcBOffset,
            uint32_t outputOffset);

        void perform(
            CommandList& cmd,
            const BufferSRV src,
            const BufferUAV inputOutput,
            BufferMathOperation op,
            uint32_t count,
            uint32_t srcOffset,
            uint32_t inputOutputOffset);

        void perform(
            CommandList& cmd,
            const BufferUAV buffer,
            BufferMathOperation op,
            uint32_t count,
            uint32_t offset,
            float value);
    private:
        engine::Pipeline<shaders::MathOperation> m_mathOperationAddition;
        engine::Pipeline<shaders::MathOperation> m_mathOperationSubtraction;
        engine::Pipeline<shaders::MathOperation> m_mathOperationDivision;
        engine::Pipeline<shaders::MathOperation> m_mathOperationMultiplication;
        engine::Pipeline<shaders::MathOperationConstant> m_mathOperationAdditionConstant;
        engine::Pipeline<shaders::MathOperationConstant> m_mathOperationSubtractionConstant;
        engine::Pipeline<shaders::MathOperationConstant> m_mathOperationDivisionConstant;
        engine::Pipeline<shaders::MathOperationConstant> m_mathOperationMultiplicationConstant;
        engine::Pipeline<shaders::MathOperationConstant> m_mathOperationSetConstant;
        engine::Pipeline<shaders::MathOperationSameOutput> m_mathOperationSameOutputAddition;
        engine::Pipeline<shaders::MathOperationSameOutput> m_mathOperationSameOutputSubtraction;
        engine::Pipeline<shaders::MathOperationSameOutput> m_mathOperationSameOutputDivision;
        engine::Pipeline<shaders::MathOperationSameOutput> m_mathOperationSameOutputMultiplication;
    };
}