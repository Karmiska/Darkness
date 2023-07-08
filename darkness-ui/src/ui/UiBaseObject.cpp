#include "ui/UiBaseObject.h"
#include "tools/Debug.h"

namespace ui
{
    void UiBaseObject::addChild(UiBaseObjectPtr child)
    {
        for (auto ch : m_childs)
            if (ch == child)
                ASSERT(false, "tried adding child to object that has it already");

        if (child->parent())
            child->reparent(nullptr);

        if (child->m_alwaysOntop)
            m_childs.emplace_back(child);
        else
        {
            bool added = false;
            for (int i = m_childs.size() - 1; i >= 0;)
            {
                if (m_childs[i]->m_alwaysOntop)
                    --i;
                else
                {
                    m_childs.insert(m_childs.begin() + i + 1, child);
                    added = true;
                    break;
                }
            }
            if(!added)
                m_childs.insert(m_childs.begin(), child);
        }
            

        child->setParent(this);
        onAddChild(child);
        onChildsChanged();
    }

    void UiBaseObject::removeChild(UiBaseObjectPtr child)
    {
        onRemoveChild(child);
        auto toRemove = std::find(m_childs.begin(), m_childs.end(), child);
        if (toRemove != m_childs.end())
            m_childs.erase(toRemove);
        onChildsChanged();
    }

    void UiBaseObject::reparent(UiBaseObjectPtr parent)
    {
        auto temp = shared_from_this();
        if (m_parent)
        {
            m_parent->removeChild(temp);
        }
        if (parent)
        {
            m_parent = parent.get();
            parent->addChild(temp);
        }
        else
            m_parent = nullptr;
    }

    void UiBaseObject::setParent(UiBaseObject* parent)
    {
        m_parent = parent;
    }

    const UiBaseObject* UiBaseObject::parent() const
    {
        return m_parent;
    }

    UiBaseObject* UiBaseObject::parent()
    {
        return m_parent;
    }

    const engine::vector<UiBaseObjectPtr>& UiBaseObject::childs() const
    {
        return m_childs;
    }

    engine::vector<UiBaseObjectPtr>& UiBaseObject::childs()
    {
        return m_childs;
    }

    bool UiBaseObject::alwaysOntop() const
    {
        return m_alwaysOntop;
    }

    void UiBaseObject::alwaysOntop(bool val)
    {
        /*if (m_parent && val)
        {
            if (m_parent->childs().size() > 0)
            {
                for (int i = 0; i < m_parent->childs().size() - 1; ++i)
                {
                    if (m_parent->childs()[i].get() == this)
                    {
                        auto temp = m_parent->childs()[i];
                        m_parent->childs().erase(m_parent->childs().begin() + i);
                        m_parent->childs().emplace_back(temp);
                    }
                }
            }
            else
                m_parent->childs().emplace_back(this);
        }*/
        m_alwaysOntop = val;
    }

    UiBaseObjectPtr UiBaseObject::locate(UiBaseObject* ptr)
    {
        for (int i = 0; i < m_childs.size(); ++i)
            if (m_childs[i].get() == ptr)
                return m_childs[i];
        return nullptr;
    }
}
