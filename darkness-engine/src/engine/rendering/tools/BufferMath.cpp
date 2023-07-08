#include "engine/rendering/tools/BufferMath.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    BufferMath::BufferMath(Device& device)
        : m_mathOperationAddition{ device.createPipeline<shaders::MathOperation>() }
        , m_mathOperationSubtraction{ device.createPipeline<shaders::MathOperation>() }
        , m_mathOperationDivision{ device.createPipeline<shaders::MathOperation>() }
        , m_mathOperationMultiplication{ device.createPipeline<shaders::MathOperation>() }
        , m_mathOperationAdditionConstant{ device.createPipeline<shaders::MathOperationConstant>() }
        , m_mathOperationSubtractionConstant{ device.createPipeline<shaders::MathOperationConstant>() }
        , m_mathOperationDivisionConstant{ device.createPipeline<shaders::MathOperationConstant>() }
        , m_mathOperationMultiplicationConstant{ device.createPipeline<shaders::MathOperationConstant>() }
        , m_mathOperationSetConstant{ device.createPipeline<shaders::MathOperationConstant>() }
        , m_mathOperationSameOutputAddition{ device.createPipeline<shaders::MathOperationSameOutput>() }
        , m_mathOperationSameOutputSubtraction{ device.createPipeline<shaders::MathOperationSameOutput>() }
        , m_mathOperationSameOutputDivision{ device.createPipeline<shaders::MathOperationSameOutput>() }
        , m_mathOperationSameOutputMultiplication{ device.createPipeline<shaders::MathOperationSameOutput>() }
    {
        m_mathOperationAddition.cs.operation = shaders::MathOperationCS::Operation::Addition;
        m_mathOperationSubtraction.cs.operation = shaders::MathOperationCS::Operation::Subtraction;
        m_mathOperationDivision.cs.operation = shaders::MathOperationCS::Operation::Division;
        m_mathOperationMultiplication.cs.operation = shaders::MathOperationCS::Operation::Multiplication;

        m_mathOperationAdditionConstant.cs.operation = shaders::MathOperationConstantCS::Operation::Addition;
        m_mathOperationSubtractionConstant.cs.operation = shaders::MathOperationConstantCS::Operation::Subtraction;
        m_mathOperationDivisionConstant.cs.operation = shaders::MathOperationConstantCS::Operation::Division;
        m_mathOperationMultiplicationConstant.cs.operation = shaders::MathOperationConstantCS::Operation::Multiplication;
        m_mathOperationSetConstant.cs.operation = shaders::MathOperationConstantCS::Operation::Set;

        m_mathOperationSameOutputAddition.cs.operation = shaders::MathOperationSameOutputCS::Operation::Addition;
        m_mathOperationSameOutputSubtraction.cs.operation = shaders::MathOperationSameOutputCS::Operation::Subtraction;
        m_mathOperationSameOutputDivision.cs.operation = shaders::MathOperationSameOutputCS::Operation::Division;
        m_mathOperationSameOutputMultiplication.cs.operation = shaders::MathOperationSameOutputCS::Operation::Multiplication;
    }

    void BufferMath::perform(
        CommandList& cmd,
        const BufferSRV srcA,
        const BufferSRV srcB,
        BufferUAV output,
        BufferMathOperation op,
        uint32_t count,
        uint32_t srcAOffset,
        uint32_t srcBOffset,
        uint32_t outputOffset)
    {
        switch (op)
        {
            case BufferMathOperation::Addition:
            {
                m_mathOperationAddition.cs.srcA = srcA;
                m_mathOperationAddition.cs.srcB = srcB;
                m_mathOperationAddition.cs.output = output;
                m_mathOperationAddition.cs.count.x = count;
                m_mathOperationAddition.cs.srcAOffset.x = srcAOffset;
                m_mathOperationAddition.cs.srcBOffset.x = srcBOffset;
                m_mathOperationAddition.cs.outputOffset.x = outputOffset;
                cmd.bindPipe(m_mathOperationAddition);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Subtraction:
            {
                m_mathOperationSubtraction.cs.srcA = srcA;
                m_mathOperationSubtraction.cs.srcB = srcB;
                m_mathOperationSubtraction.cs.output = output;
                m_mathOperationSubtraction.cs.count.x = count;
                m_mathOperationSubtraction.cs.srcAOffset.x = srcAOffset;
                m_mathOperationSubtraction.cs.srcBOffset.x = srcBOffset;
                m_mathOperationSubtraction.cs.outputOffset.x = outputOffset;
                cmd.bindPipe(m_mathOperationSubtraction);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Division:
            {
                m_mathOperationDivision.cs.srcA = srcA;
                m_mathOperationDivision.cs.srcB = srcB;
                m_mathOperationDivision.cs.output = output;
                m_mathOperationDivision.cs.count.x = count;
                m_mathOperationDivision.cs.srcAOffset.x = srcAOffset;
                m_mathOperationDivision.cs.srcBOffset.x = srcBOffset;
                m_mathOperationDivision.cs.outputOffset.x = outputOffset;
                cmd.bindPipe(m_mathOperationDivision);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Multiplication:
            {
                m_mathOperationMultiplication.cs.srcA = srcA;
                m_mathOperationMultiplication.cs.srcB = srcB;
                m_mathOperationMultiplication.cs.output = output;
                m_mathOperationMultiplication.cs.count.x = count;
                m_mathOperationMultiplication.cs.srcAOffset.x = srcAOffset;
                m_mathOperationMultiplication.cs.srcBOffset.x = srcBOffset;
                m_mathOperationMultiplication.cs.outputOffset.x = outputOffset;
                cmd.bindPipe(m_mathOperationMultiplication);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Set:
            {
                ASSERT(false, "BufferMath invalid operation. Setting a value does not make sense");
                break;
            }
        }
    }

    void BufferMath::perform(
        CommandList& cmd,
        const BufferSRV src,
        const BufferUAV inputOutput,
        BufferMathOperation op,
        uint32_t count,
        uint32_t srcOffset,
        uint32_t inputOutputOffset)
    {
        switch (op)
        {
            case BufferMathOperation::Addition:
            {
                m_mathOperationSameOutputAddition.cs.src = src;
                m_mathOperationSameOutputAddition.cs.inputOutput = inputOutput;
                m_mathOperationSameOutputAddition.cs.count.x = count;
                m_mathOperationSameOutputAddition.cs.srcOffset.x = srcOffset;
                m_mathOperationSameOutputAddition.cs.inputOutputOffset.x = inputOutputOffset;
                cmd.bindPipe(m_mathOperationSameOutputAddition);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Subtraction:
            {
                m_mathOperationSameOutputSubtraction.cs.src = src;
                m_mathOperationSameOutputSubtraction.cs.inputOutput = inputOutput;
                m_mathOperationSameOutputSubtraction.cs.count.x = count;
                m_mathOperationSameOutputSubtraction.cs.srcOffset.x = srcOffset;
                m_mathOperationSameOutputSubtraction.cs.inputOutputOffset.x = inputOutputOffset;
                cmd.bindPipe(m_mathOperationSameOutputSubtraction);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Division:
            {
                m_mathOperationSameOutputDivision.cs.src = src;
                m_mathOperationSameOutputDivision.cs.inputOutput = inputOutput;
                m_mathOperationSameOutputDivision.cs.count.x = count;
                m_mathOperationSameOutputDivision.cs.srcOffset.x = srcOffset;
                m_mathOperationSameOutputDivision.cs.inputOutputOffset.x = inputOutputOffset;
                cmd.bindPipe(m_mathOperationSameOutputDivision);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Multiplication:
            {
                m_mathOperationSameOutputMultiplication.cs.src = src;
                m_mathOperationSameOutputMultiplication.cs.inputOutput = inputOutput;
                m_mathOperationSameOutputMultiplication.cs.count.x = count;
                m_mathOperationSameOutputMultiplication.cs.srcOffset.x = srcOffset;
                m_mathOperationSameOutputMultiplication.cs.inputOutputOffset.x = inputOutputOffset;
                cmd.bindPipe(m_mathOperationSameOutputMultiplication);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Set:
            {
                ASSERT(false, "BufferMath invalid operation. Setting a value does not make sense");
                break;
            }
        }
    }

    void BufferMath::perform(
        CommandList& cmd,
        const BufferUAV buffer,
        BufferMathOperation op,
        uint32_t count,
        uint32_t offset,
        float value)
    {
        switch (op)
        {
            case BufferMathOperation::Addition:
            {
                m_mathOperationAdditionConstant.cs.operationBuffer = buffer;
                m_mathOperationAdditionConstant.cs.count.x = count;
                m_mathOperationAdditionConstant.cs.offset.x = offset;
                m_mathOperationAdditionConstant.cs.value = value;
                cmd.bindPipe(m_mathOperationAdditionConstant);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Subtraction:
            {
                m_mathOperationSubtractionConstant.cs.operationBuffer = buffer;
                m_mathOperationSubtractionConstant.cs.count.x = count;
                m_mathOperationSubtractionConstant.cs.offset.x = offset;
                m_mathOperationSubtractionConstant.cs.value = value;
                cmd.bindPipe(m_mathOperationSubtractionConstant);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Division:
            {
                m_mathOperationDivisionConstant.cs.operationBuffer = buffer;
                m_mathOperationDivisionConstant.cs.count.x = count;
                m_mathOperationDivisionConstant.cs.offset.x = offset;
                m_mathOperationDivisionConstant.cs.value = value;
                cmd.bindPipe(m_mathOperationDivisionConstant);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Multiplication:
            {
                m_mathOperationMultiplicationConstant.cs.operationBuffer = buffer;
                m_mathOperationMultiplicationConstant.cs.count.x = count;
                m_mathOperationMultiplicationConstant.cs.offset.x = offset;
                m_mathOperationMultiplicationConstant.cs.value = value;
                cmd.bindPipe(m_mathOperationMultiplicationConstant);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
            case BufferMathOperation::Set:
            {
                m_mathOperationSetConstant.cs.operationBuffer = buffer;
                m_mathOperationSetConstant.cs.count.x = count;
                m_mathOperationSetConstant.cs.offset.x = offset;
                m_mathOperationSetConstant.cs.value = value;
                cmd.bindPipe(m_mathOperationSetConstant);
                cmd.dispatch(roundUpToMultiple(count, 64) / 64, 1, 1);
                break;
            }
        }
    }
}
