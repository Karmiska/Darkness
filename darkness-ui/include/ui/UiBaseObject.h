#pragma once

#include "containers/vector.h"
#include "containers/memory.h"
#include <memory>

namespace ui
{
    class UiBaseObject;
    using UiBaseObjectPtr = engine::shared_ptr<UiBaseObject>;

    class UiBaseObject : public std::enable_shared_from_this<UiBaseObject>
    {
    public:
        virtual ~UiBaseObject() = default;

        void addChild(UiBaseObjectPtr child);
        void removeChild(UiBaseObjectPtr child);
        
        void reparent(UiBaseObjectPtr parent);

        const engine::vector<UiBaseObjectPtr>& childs() const;
        engine::vector<UiBaseObjectPtr>& childs();

        bool alwaysOntop() const;
        void alwaysOntop(bool);

    protected:
        void setParent(UiBaseObject* parent);
        const UiBaseObject* parent() const;
        UiBaseObject* parent();

        engine::vector<UiBaseObjectPtr> m_childs;
        UiBaseObject* m_parent = nullptr;
        bool m_alwaysOntop = false;
        
        UiBaseObjectPtr locate(UiBaseObject* ptr);

        virtual void onAddChild(UiBaseObjectPtr child) {};
        virtual void onRemoveChild(UiBaseObjectPtr child) {};
        virtual void onChildsChanged() {};
    };
}
