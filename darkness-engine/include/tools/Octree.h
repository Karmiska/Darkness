#pragma once

#include "engine/primitives/Vector3.h"
#include "engine/primitives/BoundingBox.h"
#include "imgui.h"
#include "containers/vector.h"
#include <algorithm>

#undef DEBUG_OCTREE

namespace engine
{
#ifdef DEBUG_OCTREE
    extern engine::vector<engine::BoundingBox> usedBoundingBoxes;
    extern engine::vector<engine::BoundingBox> searchBoundingBoxes;
    extern engine::vector<engine::BoundingBox> initialSearchBoundingBoxes;
#endif

    enum class OcNodeBoundingBox
    {
        NegXNegYNegZ,
        PosXNegYNegZ,
        NegXPosYNegZ,
        PosXPosYNegZ,
        NegXNegYPosZ,
        PosXNegYPosZ,
        NegXPosYPosZ,
        PosXPosYPosZ
    };

    using OctreePayload = uint32_t;

    enum class OctreeNodeType
    {
        Root,
        Branch,
        Leaf
    };

    template<typename Payload>
    struct OctreeEntry
    {
        Vector3f point;
        Payload payload;
    };

    template<typename Payload>
    class Octree
    {
    public:
        Octree(const BoundingBox& bb);
        Octree(Octree* parent, const BoundingBox& bb, const Vector3f& point, Payload payload);
        OctreeNodeType type() const;
        void insert(const Vector3f& point, Payload payload);
        void erase(const Vector3f& point, Payload payload);
        Payload getClosestPayload(const Vector3f& point);

        // for debugging
        engine::vector<BoundingBox> recursiveBoundingBoxes() const;
    private:
        

        void erase(const Vector3f& point, Payload payload, bool& found, int& foundIndex, bool& erased);
        void getClosestPayload(Payload& result, const Vector3f& point, bool& found, bool& measured, Payload& foundLoad, engine::BoundingBox& originalBB);
        engine::vector<OctreeEntry<Payload>> allFromBoundingBox(const engine::BoundingBox& bb, bool& foundRoot, const engine::BoundingBox& originalBB);
        engine::vector<OctreeEntry<Payload>> getAll() const;
        BoundingBox m_bb;
        engine::vector<OctreeEntry<Payload>> m_entry;
        OctreeNodeType m_type;
        Octree* m_parent;
        OcNodeBoundingBox m_leafPos;

        void updateTemporaries();
        float m_w;
        float m_h;
        float m_d;
        float m_hw;
        float m_hh;
        float m_hd;

        BoundingBox childBoundingBox(OcNodeBoundingBox node) const;

        engine::vector<engine::shared_ptr<Octree>> m_childs;
    };

    template<typename Payload>
    Octree<Payload>::Octree(const BoundingBox& bb)
        : m_bb{ bb }
        , m_entry{}
        , m_type{ OctreeNodeType::Root }
        , m_parent{ nullptr }
    {
        updateTemporaries();
    }

    template<typename Payload>
    Octree<Payload>::Octree(Octree* parent, const BoundingBox& bb, const Vector3f& point, Payload payload)
        : m_bb{ bb }
        , m_entry{}
        , m_type{ OctreeNodeType::Leaf }
        , m_parent{ parent }
    {
        m_entry.emplace_back(OctreeEntry<Payload>{ point, payload });
        updateTemporaries();
        for (int i = 0; i < 8; ++i)
        {
            auto b = childBoundingBox(static_cast<OcNodeBoundingBox>(i));
            if (b.contains(point))
            {
                m_leafPos = static_cast<OcNodeBoundingBox>(i);
                break;
            }
        }
    }

    template<typename Payload>
    OctreeNodeType Octree<Payload>::type() const
    {
        return m_type;
    }

    template<typename Payload>
    void Octree<Payload>::updateTemporaries()
    {
        m_w = m_bb.width();
        m_h = m_bb.height();
        m_d = m_bb.depth();

        m_hw = m_bb.width() / 2.0f;
        m_hh = m_bb.height() / 2.0f;
        m_hd = m_bb.depth() / 2.0f;
    }

    /*template<typename Payload>
    const Vector3f& Octree<Payload>::point() const
    {
        return m_entry.point;
    }*/

    template<typename Payload>
    engine::vector<BoundingBox> Octree<Payload>::recursiveBoundingBoxes() const
    {
        engine::vector<BoundingBox> res;
        res.emplace_back(m_bb);
        for (auto&& child : m_childs)
        {
            auto r = child->recursiveBoundingBoxes();
            res.insert(res.end(), r.begin(), r.end());
        }
        return res;
    }

    template<typename Payload>
    Payload Octree<Payload>::getClosestPayload(const Vector3f& point)
    {
        Payload result = -1;
        bool found = false;
        bool measured = false;
        Payload foundLoad;
        engine::BoundingBox originalBB;
        getClosestPayload(result, point, found, measured, foundLoad, originalBB);
        return result;
    }

    template<typename Payload>
    engine::vector<OctreeEntry<Payload>> Octree<Payload>::allFromBoundingBox(const engine::BoundingBox& bb, bool& foundRoot, const engine::BoundingBox& originalBB)
    {
        if(m_type != OctreeNodeType::Root && !foundRoot)
        {
            ASSERT(m_parent != nullptr, "Cant find root from Octree!");
            return m_parent->allFromBoundingBox(bb, foundRoot, originalBB);
        }

        if (m_type == OctreeNodeType::Root)
        {
            foundRoot = true;

            engine::vector<OctreeEntry<Payload>> res;
            for (auto&& child : m_childs)
            {
                auto un = child->m_bb.bb_intersection(bb);
                if (un.valid())
                {
#ifdef DEBUG_OCTREE
                    searchBoundingBoxes.emplace_back(un);
#endif
                    auto r = child->allFromBoundingBox(un, foundRoot, originalBB);
                    res.insert(res.end(), r.begin(), r.end());
                }
            }
            return res;
        }

        if (m_type != OctreeNodeType::Root)
        {
            if ((m_type == OctreeNodeType::Branch) || (m_type == OctreeNodeType::Root))
            {
                engine::vector<OctreeEntry<Payload>> res;
                for (auto&& child : m_childs)
                {
                    auto un = child->m_bb.bb_intersection(bb);
                    if (un.valid())
                    {
#ifdef DEBUG_OCTREE
                        searchBoundingBoxes.emplace_back(un);
#endif
                        auto r = child->allFromBoundingBox(un, foundRoot, originalBB);
                        res.insert(res.end(), r.begin(), r.end());
                    }
                }
                return res;
            }
            else if (m_type == OctreeNodeType::Leaf)
            {
                float temp = originalBB.width() > originalBB.height() ? originalBB.height() : originalBB.width();
                auto minsize = temp > originalBB.depth() ? originalBB.depth() : temp;
                minsize /= 2.0f;
                
                engine::vector<OctreeEntry<Payload>> res;
                for (auto&& entry : m_entry)
                {
                    if (bb.contains(entry.point) && (originalBB.center() - entry.point).magnitude() <= minsize)
                    {
#ifdef DEBUG_OCTREE
                        usedBoundingBoxes.emplace_back(bb);
#endif
                        res.emplace_back(OctreeEntry<Payload>{ entry });
                    }
                }
                return res;;
            }
        }
        return {};
    }

    template<typename Payload>
    void Octree<Payload>::erase(const Vector3f& point, Payload payload)
    {
        bool found = false;
        bool erased = false;
        int foundEntry = -1;
        erase(point, payload, found, foundEntry, erased);
    }

    template<typename Payload>
    void Octree<Payload>::erase(const Vector3f& point, Payload payload, bool& found, int& foundIndex, bool& erased)
    {
        if (m_type == OctreeNodeType::Leaf)
        {
            //for (auto&& entry : m_entry)
            for(int i = 0; i < m_entry.size(); ++i)
            {
                if (m_entry[i].payload == payload)
                {
                    foundIndex = i;
                    found = true;
                    return;
                }
            }
        }

        for (int i = 0; i < m_childs.size(); ++i)
        {
            if (m_childs[i]->m_bb.contains(point))
            {
                m_childs[i]->erase(point, payload, found, foundIndex, erased);
                if (found && !erased)
                {
                    m_childs[i]->m_entry.erase(m_childs[i]->m_entry.begin() + foundIndex);
                    if (m_childs[i]->m_entry.size() == 0)
                    {
                        m_childs.erase(m_childs.begin() + i);

                        if (m_childs.size() == 1 &&
                            m_childs[0]->m_type == OctreeNodeType::Leaf)
                        {
                            if (m_type == OctreeNodeType::Branch)
                            {
                                m_entry.insert(m_entry.end(), m_childs[0]->m_entry.begin(), m_childs[0]->m_entry.end());
                                m_type = OctreeNodeType::Leaf;
                            }
                            m_childs.clear();
                        }
                    }
                    erased = true;
                    return;
                }
            }
        }
    }

    template<typename Payload>
    engine::vector<OctreeEntry<Payload>> Octree<Payload>::getAll() const
    {
        if (m_childs.size() == 0 && m_entry.size() == 0)
            return {};
        else if (m_childs.size() == 0)
            return m_entry;

        engine::vector<OctreeEntry<Payload>> res;
        res.insert(res.end(), m_entry.begin(), m_entry.end());
        for (auto&& child : m_childs)
        {
            auto r = child->getAll();
            res.insert(res.end(), r.begin(), r.end());
        }
        return res;
    }

    template<typename Payload>
    void Octree<Payload>::getClosestPayload(Payload& result, const Vector3f& point, bool& found, bool& measured, Payload& foundLoad, engine::BoundingBox& originalBB)
    {
        if (m_type == OctreeNodeType::Leaf)
        {
            for (auto&& entry : m_entry)
            {
                if (entry.point == point)
                {
                    foundLoad = entry.payload;
                    found = true;

                    auto bbmax = std::max(std::max(m_bb.width(), m_bb.height()), m_bb.depth());
                    bool foundRoot = false;
                    originalBB = BoundingBox(point, bbmax);
                    auto list = allFromBoundingBox(originalBB, foundRoot, originalBB);

#ifdef DEBUG_OCTREE
                    initialSearchBoundingBoxes.emplace_back(BoundingBox(point, bbmax));
#endif

                    if (list.size() > 0)
                    {
                        float distance = std::numeric_limits<float>::max();
                        Payload pload = {};
                        for (int aa = 0; aa < list.size(); ++aa)
                        {
                            if (list[aa].payload != foundLoad)
                            {
                                auto mag = (point - list[aa].point).magnitude();
                                if (mag < distance)
                                {
                                    distance = mag;
                                    pload = list[aa].payload;
                                }
                                measured = true;
                            }
                        }
                        result = pload;
                        return;
                    }
                    else
                        return;
                }
            }
            return;
        }

        if (m_childs.size() > 0)
        {
            for (int i = 0; i < m_childs.size(); ++i)
            {
                if (!measured && m_childs[i]->m_bb.contains(point))
                {
                    m_childs[i]->getClosestPayload(result, point, found, measured, foundLoad, originalBB);

                    if (found && !measured)
                    {
                        auto bbmax = std::max(std::max(m_bb.width(), m_bb.height()), m_bb.depth());
                        bool foundRoot = false;
                        auto list = allFromBoundingBox(BoundingBox(originalBB.center(), bbmax), foundRoot, BoundingBox(originalBB.center(), bbmax));

                        if (list.size() > 0)
                        {
                            float distance = std::numeric_limits<float>::max();
                            Payload pload = {};
                            for (int aa = 0; aa < list.size(); ++aa)
                            {
                                if (list[aa].payload != foundLoad)
                                {
                                    auto mag = (point - list[aa].point).magnitude();
                                    if (mag < distance)
                                    {
                                        distance = mag;
                                        pload = list[aa].payload;
                                    }
                                    measured = true;
                                }
                            }
                            result = pload;
                            return;
                        }
                        else
                            return;
                        
                    }
                }
                if (measured)
                    break;
            }
        }
        else if(m_entry.size() > 0)
            result = m_entry[0].payload;
    }

    template<typename Payload>
    BoundingBox Octree<Payload>::childBoundingBox(OcNodeBoundingBox node) const
    {
        switch (node)
        {
        case OcNodeBoundingBox::NegXNegYNegZ: return BoundingBox{ float3{ m_bb.min.x, m_bb.min.y, m_bb.min.z }, float3{ m_bb.min.x + m_hw, m_bb.min.y + m_hh, m_bb.min.z + m_hd } };
        case OcNodeBoundingBox::PosXNegYNegZ: return BoundingBox{ float3{ nextafterf(m_bb.min.x + m_hw, m_bb.min.x + m_hw + 1.0f), m_bb.min.y, m_bb.min.z }, float3{ m_bb.min.x + m_w, m_bb.min.y + m_hh, m_bb.min.z + m_hd } };
        case OcNodeBoundingBox::NegXPosYNegZ: return BoundingBox{ float3{ m_bb.min.x, nextafterf(m_bb.min.y + m_hh, m_bb.min.y + m_hh + 1.0f), m_bb.min.z }, float3{ m_bb.min.x + m_hw, m_bb.min.y + m_h, m_bb.min.z + m_hd } };
        case OcNodeBoundingBox::PosXPosYNegZ: return BoundingBox{ float3{ nextafterf(m_bb.min.x + m_hw, m_bb.min.x + m_hw + 1.0f), nextafterf(m_bb.min.y + m_hh, m_bb.min.y + m_hh + 1.0f), m_bb.min.z }, float3{ m_bb.min.x + m_w, m_bb.min.y + m_h, m_bb.min.z + m_hd } };
        case OcNodeBoundingBox::NegXNegYPosZ: return BoundingBox{ float3{ m_bb.min.x, m_bb.min.y, nextafterf(m_bb.min.z + m_hd, m_bb.min.z + m_hd + 1.0f) }, float3{ m_bb.min.x + m_hw, m_bb.min.y + m_hh, m_bb.min.z + m_d } };
        case OcNodeBoundingBox::PosXNegYPosZ: return BoundingBox{ float3{ nextafterf(m_bb.min.x + m_hw, m_bb.min.x + m_hw + 1.0f), m_bb.min.y, nextafterf(m_bb.min.z + m_hd, m_bb.min.z + m_hd + 1.0f) }, float3{ m_bb.min.x + m_w, m_bb.min.y + m_hh, m_bb.min.z + m_d } };
        case OcNodeBoundingBox::NegXPosYPosZ: return BoundingBox{ float3{ m_bb.min.x, nextafterf(m_bb.min.y + m_hh, m_bb.min.y + m_hh + 1.0f), nextafterf(m_bb.min.z + m_hd, m_bb.min.z + m_hd + 1.0f) }, float3{ m_bb.min.x + m_hw, m_bb.min.y + m_h, m_bb.min.z + m_d } };
        case OcNodeBoundingBox::PosXPosYPosZ: return BoundingBox{ float3{ nextafterf(m_bb.min.x + m_hw, m_bb.min.x + m_hw + 1.0f), nextafterf(m_bb.min.y + m_hh, m_bb.min.y + m_hh + 1.0f), nextafterf(m_bb.min.z + m_hd, m_bb.min.z + m_hd + 1.0f) }, float3{ m_bb.min.x + m_w, m_bb.min.y + m_h, m_bb.min.z + m_d } };
        }
        return {};
    }

    template<typename Payload>
    void Octree<Payload>::insert(const Vector3f& point, Payload payload)
    {
        ASSERT(m_bb.contains(point), "Tried to add point inside bounding box that does not contain it");

        engine::vector<engine::shared_ptr<Octree>>* childs = &m_childs;
        auto insertAsChild = [this, childs](const Vector3f& point, Payload payload)
        {
            bool inserted = false;
            for (auto&& child : *childs)
            {
                if (child->m_bb.contains(point))
                {
                    child->insert(point, payload);
                    inserted = true;
                    break;
                }
            }

            if (!inserted)
            {
                for (int i = 0; i < 8; ++i)
                {
                    auto bb = this->childBoundingBox(static_cast<OcNodeBoundingBox>(i));
                    if (bb.contains(point))
                    {
                        m_childs.emplace_back(engine::make_shared<Octree>(this, bb, point, payload));
                    }
                }
            }
        };

        if ((m_type == OctreeNodeType::Root) || (m_type == OctreeNodeType::Branch))
        {
            insertAsChild(point, payload);
        }
        else if (m_type == OctreeNodeType::Leaf)
        {
            for (int i = 0; i < 8; ++i)
            {
                auto bb = childBoundingBox(static_cast<OcNodeBoundingBox>(i));
                if (bb.contains(point))
                {
                    if (m_leafPos == static_cast<OcNodeBoundingBox>(i) && m_entry.size() < 200)
                    {
                        m_entry.emplace_back(OctreeEntry<Payload>{point, payload});
                        break;
                    }
                    else if (m_leafPos == static_cast<OcNodeBoundingBox>(i))
                    {
                        m_type = OctreeNodeType::Branch;
                        for(auto&& entry : m_entry)
                        {
                            insertAsChild(entry.point, entry.payload);
                        }
                        m_entry.clear();
                        insertAsChild(point, payload);
                        break;
                    }
                    else
                    {
                        m_type = OctreeNodeType::Branch;

                        m_childs.emplace_back(engine::make_shared<Octree>(this, childBoundingBox(m_leafPos), m_entry[0].point, m_entry[0].payload));
                        for (int a = 1; a < m_entry.size(); ++a)
                        {
                            m_childs.back()->insert(m_entry[a].point, m_entry[a].payload);
                        }
                        m_entry.clear();
                        m_childs.emplace_back(engine::make_shared<Octree>(this, bb, point, payload));
                        break;
                    }
                }
            }
        }
    }
}