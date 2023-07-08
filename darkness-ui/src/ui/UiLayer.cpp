#include "ui/UiLayer.h"
#include "tools/Debug.h"

namespace ui
{
    void UiBaseLayer::moveToTop()
    {
        if (!m_parent)
            return;

        auto& parentChilds = m_parent->childs();
        auto MoveSelfToIndex = [&](int index)
        {
            auto found = std::find(parentChilds.begin(), parentChilds.end(), shared_from_this());
            if (found == parentChilds.end())
                ASSERT(false, "Could not find object in parent!");

            auto ownSharedPtr = *found;
            parentChilds.erase(found);

            parentChilds.insert(parentChilds.begin() + index, ownSharedPtr);
        };

        if (alwaysOntop())
        {
            MoveSelfToIndex(parentChilds.size()-1);
        }
        else
        {
            auto IndexOfBeginningOfAlwaysOntop = [&]()->size_t
            {
                for (int i = 0; i < parentChilds.size(); ++i)
                    if (parentChilds[i]->alwaysOntop())
                        return std::max(i-1, 0);
                return parentChilds.size()-1;
            };

            auto alwaysOntopIndex = IndexOfBeginningOfAlwaysOntop();
            MoveSelfToIndex(alwaysOntopIndex);
        }

        

        


        /*for (int i = 0; i < parentChilds.size() - 1; ++i)
        {
            if (parentChilds[i].get() == this)
            {
                if (alwaysOntop())
                {
                    auto temp = parentChilds[i];
                    parentChilds.erase(parentChilds.begin() + i);
                    parentChilds.emplace_back(temp);
                }
                else
                {
                    for (int a = parentChilds.size() - 1; a >= 0;)
                    {
                        if (parentChilds[a]->alwaysOntop())
                            --a;
                        else
                        {
                            auto temp = parentChilds[i];
                            parentChilds.erase(parentChilds.begin() + i);
                            parentChilds.insert(parentChilds.begin() + a + 1, temp);
                            return;
                        }
                    }

                    auto temp = parentChilds[i];
                    parentChilds.erase(parentChilds.begin() + i);
                    parentChilds.emplace_back(temp);
                    return;
                }
            }
        }*/
    }

    void UiBaseLayer::moveUp()
    {
        if (!m_parent)
            return;

        for (int i = 0; i < m_parent->childs().size() - 1; ++i)
        {
            if (m_parent->childs()[i].get() == this)
            {
                if(alwaysOntop())
                {
                    auto temp = m_parent->childs()[i];
                    m_parent->childs().erase(m_parent->childs().begin() + i);
                    m_parent->childs().insert(m_parent->childs().begin() + i + 1, temp);
                }
                else
                {
                    if (i + 1 < m_parent->childs().size() && m_parent->childs()[i + 1]->alwaysOntop())
                        break;
                    auto temp = m_parent->childs()[i];
                    m_parent->childs().erase(m_parent->childs().begin() + i);
                    m_parent->childs().insert(m_parent->childs().begin() + i + 1, temp);
                }
            }
        }
    }

    void UiBaseLayer::moveDown()
    {
        if (!m_parent)
            return;

        for (int i = 1; i < m_parent->childs().size(); ++i)
        {
            if (m_parent->childs()[i].get() == this)
            {
                auto temp = m_parent->childs()[i];
                m_parent->childs().erase(m_parent->childs().begin() + i);
                m_parent->childs().insert(m_parent->childs().begin() + i - 1, temp);
            }
        }
    }

    void UiBaseLayer::moveToBottom()
    {
        if (!m_parent)
            return;

        for (int i = 1; i < m_parent->childs().size(); ++i)
        {
            if (m_parent->childs()[i].get() == this)
            {
                auto temp = m_parent->childs()[i];
                m_parent->childs().erase(m_parent->childs().begin() + i);
                m_parent->childs().insert(m_parent->childs().begin(), temp);
            }
        }
        
    }
}
