#pragma once

#if 0
#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <any>
#include <tuple>
#include <execution>

#define PARALLEL_FOR_LOOP

namespace engine
{
    class Entity;
    class EntityStorage;

    struct ComponentSystem
    {
        size_t system;
        size_t component;
    };

    template<typename T>
    class System
    {
    public:
        std::vector<T> data;
        std::vector<T> freeData;

        size_t allocate()
        {
            auto current = data.size();
            data.emplace_back(T{});
            return current;
        };

        void free(size_t id)
        {
            data[id] = T{};
            freeData.emplace_back(id);
        };
    };

    class Ecs
    {
    private:
#if 0
#if 1
        // these are for finding the system id:s based on type
        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackSystems(std::vector<int32_t> & result) { }

        template<typename T, typename... Rest>
        void unpackSystems(std::vector<int32_t>& result)
        {
            result.emplace_back(locateSystem<std::remove_reference<T>::type>());
            unpackSystems<Rest& ...>(result);
        }
#endif

#if 1
        // these are for calling the function with tuple
        template<typename F, typename Tuple, size_t ...S >
        decltype(auto) apply_tuple_impl(F&& fn, Tuple&& t, std::index_sequence<S...>)
        {
            return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
        }
        template<typename F, typename Tuple>
        decltype(auto) apply_from_tuple(F&& fn, Tuple&& t)
        {
            std::size_t constexpr tSize
                = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
            return apply_tuple_impl(std::forward<F>(fn),
                std::forward<Tuple>(t),
                std::make_index_sequence<tSize>());
        }
#endif

        template<typename T>
        std::tuple<T&> value(int32_t system, size_t location)
        {
            return std::make_tuple<T&>(std::any_cast<System<T>&>(m_systems[system]).data[location]);
        }

        template<typename... Types>
        //std::tuple<Types...> createTuple(std::vector<int32_t>& systems, std::vector<size_t>& dataLocations)
        std::tuple<Types...> createTuple(int32_t systemA, size_t locationA, int32_t systemB, size_t locationB)
        {
            std::tuple<Transform&, RigidBody&> blah(
                std::any_cast<System<Transform>&>(m_systems[systemA]).data[locationA],
                std::any_cast<System<RigidBody>&>(m_systems[systemB]).data[locationB]);
            return blah;
        }
#endif

    public:
        Ecs();
        Entity createEntity();

#if 0
        template<typename... Types>
        void queryI(std::function<void(Types& ...)> func,
            std::vector<Entity>& m_entities,
            std::vector<std::any>& m_systems)
        {
            std::vector<int32_t> systems;
            unpackSystems<Types& ...>(systems);

            for (auto system : systems)
                if (system == -1)
                    return;

            for (auto& entity : m_entities)
            {
                /*std::vector<size_t> componentDataLocation;
                if (entity.hasComponents(systems, componentDataLocation))
                {
                    if (systems.size() == componentDataLocation.size())
                    {
                        apply_from_tuple(func, createTuple<Types & ...>(systems, componentDataLocation));
                    }
                }*/

                int32_t sysA = 0;
                size_t locationA;
                int32_t sysB = 1;
                size_t locationB;
                entity.hasComponents(sysA, sysB, locationA, locationB);
                apply_from_tuple(func, createTuple<Types & ...>(sysA, locationA, sysB, locationB));
            }
        };

        template <typename F, typename T>
        struct function_traits : public function_traits<F, decltype(&T::operator())>
        {};

        template <typename F, typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<F, ReturnType(ClassType::*)(Args...) const>
            // we specialize for pointers to member function
        {
            enum { arity = sizeof...(Args) };
            // arity is the number of arguments.

            typedef ReturnType result_type;

            template <size_t i>
            struct arg
            {
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
                // the i-th argument is equivalent to the i-th tuple element of a tuple
                // composed of those arguments.
            };

            static void call(
                Ecs& ecs,
                F func,
                std::vector<Entity>& m_entities,
                std::vector<std::any>& m_systems)
            {
                ecs.queryI<Args...>(func, m_entities, m_systems);
            };
        };

        template<typename F>
        void query(F func)
        {
            typedef function_traits<F, decltype(func)> traits;
            traits::call(*this, func, m_entities, m_systems);
        }


#endif

#if 1
#if 0
        template<typename A> void queryN(std::function<void(A&)> func)
        {
            auto systemId = locateSystem<A>();
            if (systemId != -1)
            {
                auto& system = std::any_cast<System<A>&>(m_systems[systemId]);
#ifdef PARALLEL_FOR_LOOP
                std::for_each(
                    std::execution::par_unseq,
                    system.data.begin(),
                    system.data.end(),
                    [system, func](auto && val)
#else
                for (auto val : system.data)
#endif
                {
                    func(val);
#ifdef PARALLEL_FOR_LOOP
                });
#else
            }
#endif
        }
    };
#endif
        template<typename A, typename B>
        void queryN(std::function<void(A, B)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();

            if (systemsA == -1 || systemsB == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, func, aptr, bptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, dataLocationA, dataLocationB))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };

#if 1
        template<typename A, typename B, typename C>
        void queryN(std::function<void(A, B, C)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();
            auto systemsC = locateSystem<typename std::remove_reference<C>::type>();

            if (systemsA == -1 || systemsB == -1 || systemsC == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];
            auto cptr = &std::any_cast<System<typename std::remove_reference<C>::type>&>(m_systems[systemsC]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, systemsC, func, aptr, bptr, cptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                size_t dataLocationC = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, systemsC, dataLocationA, dataLocationB, dataLocationC))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB),
                        *(cptr + dataLocationC));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };

        template<typename A, typename B, typename C, typename D>
        void queryN(std::function<void(A, B, C, D)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();
            auto systemsC = locateSystem<typename std::remove_reference<C>::type>();
            auto systemsD = locateSystem<typename std::remove_reference<D>::type>();

            if (systemsA == -1 || systemsB == -1 || systemsC == -1 || systemsD == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];
            auto cptr = &std::any_cast<System<typename std::remove_reference<C>::type>&>(m_systems[systemsC]).data[0];
            auto dptr = &std::any_cast<System<typename std::remove_reference<D>::type>&>(m_systems[systemsD]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, systemsC, systemsD, func, aptr, bptr, cptr, dptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                size_t dataLocationC = static_cast<size_t>(-1);
                size_t dataLocationD = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, systemsC, systemsD, dataLocationA, dataLocationB, dataLocationC, dataLocationD))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB),
                        *(cptr + dataLocationC),
                        *(dptr + dataLocationD));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };
        template<typename A, typename B, typename C, typename D, typename E>
        void queryN(std::function<void(A, B, C, D, E)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();
            auto systemsC = locateSystem<typename std::remove_reference<C>::type>();
            auto systemsD = locateSystem<typename std::remove_reference<D>::type>();
            auto systemsE = locateSystem<typename std::remove_reference<E>::type>();

            if (systemsA == -1 || systemsB == -1 || systemsC == -1 || systemsD == -1 || systemsE == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];
            auto cptr = &std::any_cast<System<typename std::remove_reference<C>::type>&>(m_systems[systemsC]).data[0];
            auto dptr = &std::any_cast<System<typename std::remove_reference<D>::type>&>(m_systems[systemsD]).data[0];
            auto eptr = &std::any_cast<System<typename std::remove_reference<E>::type>&>(m_systems[systemsE]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, systemsC, systemsD, systemsE, func, aptr, bptr, cptr, dptr, eptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                size_t dataLocationC = static_cast<size_t>(-1);
                size_t dataLocationD = static_cast<size_t>(-1);
                size_t dataLocationE = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, systemsC, systemsD, systemsE, dataLocationA, dataLocationB, dataLocationC, dataLocationD, dataLocationE))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB),
                        *(cptr + dataLocationC),
                        *(dptr + dataLocationD),
                        *(eptr + dataLocationE));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };
        template<typename A, typename B, typename C, typename D, typename E, typename F>
        void queryN(std::function<void(A, B, C, D, E, F)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();
            auto systemsC = locateSystem<typename std::remove_reference<C>::type>();
            auto systemsD = locateSystem<typename std::remove_reference<D>::type>();
            auto systemsE = locateSystem<typename std::remove_reference<E>::type>();
            auto systemsF = locateSystem<typename std::remove_reference<F>::type>();

            if (systemsA == -1 || systemsB == -1 || systemsC == -1 || systemsD == -1 || systemsE == -1 || systemsF == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];
            auto cptr = &std::any_cast<System<typename std::remove_reference<C>::type>&>(m_systems[systemsC]).data[0];
            auto dptr = &std::any_cast<System<typename std::remove_reference<D>::type>&>(m_systems[systemsD]).data[0];
            auto eptr = &std::any_cast<System<typename std::remove_reference<E>::type>&>(m_systems[systemsE]).data[0];
            auto fptr = &std::any_cast<System<typename std::remove_reference<F>::type>&>(m_systems[systemsF]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, systemsC, systemsD, systemsE, systemsF, func, aptr, bptr, cptr, dptr, eptr, fptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                size_t dataLocationC = static_cast<size_t>(-1);
                size_t dataLocationD = static_cast<size_t>(-1);
                size_t dataLocationE = static_cast<size_t>(-1);
                size_t dataLocationF = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, systemsC, systemsD, systemsE, systemsF, dataLocationA, dataLocationB, dataLocationC, dataLocationD, dataLocationE, dataLocationF))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB),
                        *(cptr + dataLocationC),
                        *(dptr + dataLocationD),
                        *(eptr + dataLocationE),
                        *(fptr + dataLocationF));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };
        template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
        void queryN(std::function<void(A&, B&, C&, D&, E&, F&, G&)> func)
        {
            auto systemsA = locateSystem<typename std::remove_reference<A>::type>();
            auto systemsB = locateSystem<typename std::remove_reference<B>::type>();
            auto systemsC = locateSystem<typename std::remove_reference<C>::type>();
            auto systemsD = locateSystem<typename std::remove_reference<D>::type>();
            auto systemsE = locateSystem<typename std::remove_reference<E>::type>();
            auto systemsF = locateSystem<typename std::remove_reference<F>::type>();
            auto systemsG = locateSystem<typename std::remove_reference<G>::type>();

            if (systemsA == -1 || systemsB == -1 || systemsC == -1 || systemsD == -1 || systemsE == -1 || systemsF == -1 || systemsG == -1)
                return;

            auto aptr = &std::any_cast<System<typename std::remove_reference<A>::type>&>(m_systems[systemsA]).data[0];
            auto bptr = &std::any_cast<System<typename std::remove_reference<B>::type>&>(m_systems[systemsB]).data[0];
            auto cptr = &std::any_cast<System<typename std::remove_reference<C>::type>&>(m_systems[systemsC]).data[0];
            auto dptr = &std::any_cast<System<typename std::remove_reference<D>::type>&>(m_systems[systemsD]).data[0];
            auto eptr = &std::any_cast<System<typename std::remove_reference<E>::type>&>(m_systems[systemsE]).data[0];
            auto fptr = &std::any_cast<System<typename std::remove_reference<F>::type>&>(m_systems[systemsF]).data[0];
            auto gptr = &std::any_cast<System<typename std::remove_reference<G>::type>&>(m_systems[systemsG]).data[0];

#ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [systemsA, systemsB, systemsC, systemsD, systemsE, systemsF, systemsG, func, aptr, bptr, cptr, dptr, eptr, fptr, gptr](auto && entity)
#else
            for (auto& entity : m_entities)
#endif
            {
                size_t dataLocationA = static_cast<size_t>(-1);
                size_t dataLocationB = static_cast<size_t>(-1);
                size_t dataLocationC = static_cast<size_t>(-1);
                size_t dataLocationD = static_cast<size_t>(-1);
                size_t dataLocationE = static_cast<size_t>(-1);
                size_t dataLocationF = static_cast<size_t>(-1);
                size_t dataLocationG = static_cast<size_t>(-1);
                if (entity.hasComponents(systemsA, systemsB, systemsC, systemsD, systemsE, systemsF, systemsG, dataLocationA, dataLocationB, dataLocationC, dataLocationD, dataLocationE, dataLocationF, dataLocationG))
                {
                    func(
                        *(aptr + dataLocationA),
                        *(bptr + dataLocationB),
                        *(cptr + dataLocationC),
                        *(dptr + dataLocationD),
                        *(eptr + dataLocationE),
                        *(fptr + dataLocationF),
                        *(gptr + dataLocationG));
                }
#ifdef PARALLEL_FOR_LOOP
            });
#else
        };
#endif
        };
#endif


        template <typename F, typename T>
        struct function_traits : public function_traits<F, decltype(&T::operator())>
        {};

        template <typename F, typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<F, ReturnType(ClassType::*)(Args...) const>
            // we specialize for pointers to member function
        {
            enum { arity = sizeof...(Args) };
            // arity is the number of arguments.

            typedef ReturnType result_type;

            template <size_t i>
            struct arg
            {
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
                // the i-th argument is equivalent to the i-th tuple element of a tuple
                // composed of those arguments.
            };

            static void call(
                Ecs& ecs,
                F func)
            {
                ecs.queryN<Args...>(func);
            };
        };

        template<typename F>
        void query(F func)
        {
            typedef function_traits<F, decltype(func)> traits;
            traits::call(*this, func);
        }


        template<typename T>
        System<T>& system()
        {
            for (auto& system : m_systems)
            {
                if (system.type() == typeid(System<T>))
                {
                    return std::any_cast<System<T>&>(system);
                }
            }
            // new component type
            m_systems.emplace_back(std::any(System<T>()));
            return std::any_cast<System<T>&>(m_systems.back());
        }

#endif

    private:
        std::vector<EntityStorage> m_entities;
        std::vector<std::any> m_systems;

        template<typename T>
        int32_t locateSystem()
        {
            int32_t index = 0;
            for (auto& system : m_systems)
            {
                if (system.type() == typeid(System<T>))
                {
                    //auto& sys = std::any_cast<System<T>&>(system);
                    return index;
                }
                ++index;
            }
            // ASSERT HERE
            return -1;
        }
    private:
        friend class Entity;
        friend class EntityStorage;

        template<typename T>
        ComponentSystem allocate()
        {
            size_t index = 0;
            for (auto& system : m_systems)
            {
                if (system.type() == typeid(System<T>))
                {
                    auto& sys = std::any_cast<System<T>&>(system);
                    auto current = sys.allocate();
                    return { index, current };
                }
                ++index;
            }
            // new component type
            m_systems.emplace_back(std::any(System<T>()));
            auto& sys = std::any_cast<System<T>&>(m_systems.back());
            auto current = sys.allocate();
            return { index, current };
        };

        template<typename T>
        void deallocate(ComponentSystem s)
        {
            std::any_cast<System<T>&>(m_systems[s.system]).free(s.component);
        };
    };

    class EntityStorage
    {
    public:
        EntityStorage(Ecs& ecs)
            : m_ecs{ ecs }
        {};

        template<typename T>
        void addComponent()
        {
            static_assert(std::is_standard_layout<T>::value, "Must be a POD type.");
            m_components.emplace_back(m_ecs.allocate<T>());
        };

        template<typename T>
        void removeComponent()
        {
            auto com = std::find(m_components.begin(), m_components.end(), m_ecs.locateSystem<T>());
            if (com != m_components.end())
                m_components.erase(com);
        };

        bool hasComponents(std::vector<int32_t> components, std::vector<size_t>& componentLocations)
        {
            bool found = false;
            for (auto& component : components)
            {
                for (auto& com : m_components)
                {
                    if (com.system == component)
                    {
                        componentLocations.emplace_back(com.component);
                        found = true;
                    }
                }
            }
            return found;
        };

        bool hasComponents(
            int32_t a,
            size_t& ar)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
            }
            return ar != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b,
            size_t & ar, size_t & br)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
            }
            return ar != -1 && br != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c,
            size_t & ar, size_t & br, size_t & cr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
            }
            return ar != -1 && br != -1 && cr != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d,
            size_t & ar, size_t & br, size_t & cr, size_t & dr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er, size_t & fr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
                if (com.system == f) fr = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1 && fr != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er, size_t & fr, size_t & gr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
                if (com.system == f) fr = com.component;
                if (com.system == g) gr = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1 && fr != -1 && gr != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er, size_t & fr, size_t & gr, size_t & hr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
                if (com.system == f) fr = com.component;
                if (com.system == g) gr = com.component;
                if (com.system == h) hr = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1 && fr != -1 && gr != -1 && hr != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er, size_t & fr, size_t & gr, size_t & hr, size_t & ir)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
                if (com.system == f) fr = com.component;
                if (com.system == g) gr = com.component;
                if (com.system == h) hr = com.component;
                if (com.system == i) ir = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1 && fr != -1 && gr != -1 && hr != -1 && ir != -1;
        }

        bool hasComponents(
            int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i, int32_t j,
            size_t & ar, size_t & br, size_t & cr, size_t & dr, size_t & er, size_t & fr, size_t & gr, size_t & hr, size_t & ir, size_t & jr)
        {
            for (auto& com : m_components)
            {
                if (com.system == a) ar = com.component;
                if (com.system == b) br = com.component;
                if (com.system == c) cr = com.component;
                if (com.system == d) dr = com.component;
                if (com.system == e) er = com.component;
                if (com.system == f) fr = com.component;
                if (com.system == g) gr = com.component;
                if (com.system == h) hr = com.component;
                if (com.system == i) ir = com.component;
                if (com.system == j) ir = com.component;
            }
            return ar != -1 && br != -1 && cr != -1 && dr != -1 && er != -1 && fr != -1 && gr != -1 && hr != -1 && ir != -1 && jr != -1;
        }

    private:
        friend class Entity;
        Ecs& m_ecs;
        std::vector<ComponentSystem> m_components;
    };

    class Entity
    {
    public:
        Entity(Ecs* ecs, size_t entityId)
            : m_ecs{ ecs }
            , m_entityId{ entityId }
        {}


        template<typename T>
        void addComponent()
        {
            static_assert(std::is_standard_layout<T>::value, "Must be a POD type.");
            m_ecs->m_entities[m_entityId].m_components.emplace_back(m_ecs->allocate<T>());
        };

        bool hasComponents(std::vector<int32_t> components, std::vector<size_t>& componentLocations) const { return m_ecs->m_entities[m_entityId].hasComponents(components, componentLocations); }
        bool hasComponents(int32_t a,
            size_t& ar) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, ar);
        }
        bool hasComponents(int32_t a, int32_t b,
            size_t& ar, size_t& br) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, ar, br);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c,
            size_t& ar, size_t& br, size_t& cr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, ar, br, cr);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d,
            size_t& ar, size_t& br, size_t& cr, size_t& dr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, ar, br, cr, dr);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, ar, br, cr, dr, er);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er, size_t& fr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, f, ar, br, cr, dr, er, fr);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er, size_t& fr, size_t& gr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, f, g, ar, br, cr, dr, er, fr, gr);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er, size_t& fr, size_t& gr, size_t& hr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, f, g, h, ar, br, cr, dr, er, fr, gr, hr);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er, size_t& fr, size_t& gr, size_t& hr, size_t& ir) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, f, g, h, i, ar, br, cr, dr, er, fr, gr, hr, ir);
        }
        bool hasComponents(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i, int32_t j,
            size_t& ar, size_t& br, size_t& cr, size_t& dr, size_t& er, size_t& fr, size_t& gr, size_t& hr, size_t& ir, size_t& jr) const {
            return m_ecs->m_entities[m_entityId].hasComponents(a, b, c, d, e, f, g, h, i, j, ar, br, cr, dr, er, fr, gr, hr, ir, jr);
        }

    private:
        size_t m_entityId;
        Ecs* m_ecs;
    };
}
#endif