#include "components/ModelPropertiesComponent.h"

namespace engine
{
    int ModelPropertiesComponent::size() const
    {
        return m_size.value<int>();
    }

    void ModelPropertiesComponent::size(int _size)
    {
        m_size.value<int>(_size);
    }
}
