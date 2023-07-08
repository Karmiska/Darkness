#pragma once

/*
* Concatenate preprocessor tokens A and B without expanding macro definitions
* (however, if invoked from a macro, macro arguments are expanded).
*/
#define PPCAT_NX(A, B) A ## B

/*
* Concatenate preprocessor tokens A and B after macro-expanding them.
*/
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
* INTERFACE_DEFAULT_CONSTRUCTORS Implements the rule of five
* for an interface class which needs to have a virtual destructor.
*/
#define INTERFACE_DEFAULT_CONSTRUCTORS(InterfaceClass) \
virtual ~InterfaceClass() = default; \
InterfaceClass(InterfaceClass&&) = default; \
InterfaceClass& operator=(InterfaceClass&&) = default; \
InterfaceClass(const InterfaceClass&) = default; \
InterfaceClass& operator=(const InterfaceClass&) = default;

#define DEFAULT_CONSTRUCTORS(NormalClass) \
NormalClass() = default; \
~NormalClass() = default; \
NormalClass(NormalClass&&) = default; \
NormalClass& operator=(NormalClass&&) = default; \
NormalClass(const NormalClass&) = default; \
NormalClass& operator=(const NormalClass&) = default;

/*
* PIMPL_FWD generates the following declaration based on MainClass and PimplClass names
*
class MainClass;
namespace implementation
{
class PimplClass;
struct PimplClassGet
{
static const PimplClass& impl(const MainClass& value);
static PimplClass& impl(MainClass& value);
};
}
*/
#define PIMPL_FWD(MainClass, PimplClass) \
class MainClass; \
namespace implementation \
{ \
    class PimplClass; \
    struct PPCAT(PimplClass, Get) \
    { \
        static const PimplClass & impl(const MainClass & value); \
        static PimplClass & impl(MainClass & value); \
    }; \
}

#define PIMPL_FWD_BUFFER(MainClass, PimplClass) \
class MainClass; \
namespace implementation \
{ \
    class PimplClass; \
    struct PPCAT(PimplClass, Get) \
    { \
        static const PimplClass* impl(const MainClass & value); \
        static PimplClass* impl(MainClass & value); \
    }; \
}


/*
* PIMPL_FRIEND_ACCESS generates the following declaration based on PimplClass name
*
private:
friend struct implementation::SwapChainImplGet;
const implementation::SwapChainImpl& impl() const;
implementation::SwapChainImpl& impl();
tools::impl_ptr<implementation::SwapChainImpl> m_impl;
*/
#define PIMPL_FRIEND_ACCESS(PimplClass) \
private: \
friend struct PPCAT(PPCAT(implementation:,:PimplClass), Get); \
const PPCAT(implementation:,:PimplClass) & impl() const; \
PPCAT(implementation:,:PimplClass) & impl(); \
tools::unique_impl_ptr<PPCAT(implementation:,:PimplClass)> m_impl;

#define PIMPL_FRIEND_ACCESS_BASE_BUFFER(PimplClass) \
protected: \
friend struct PPCAT(PPCAT(implementation:,:PimplClass), Get); \
const PPCAT(implementation:,:PimplClass)* impl() const; \
PPCAT(implementation:,:PimplClass)* impl(); \
tools::impl_ptr<PPCAT(implementation:,:PimplClass)> m_impl;

#define PIMPL_FRIEND_ACCESS_BUFFER(PimplClass) \
private: \
friend struct PPCAT(PPCAT(implementation:,:PimplClass), Get); \
const PPCAT(implementation:,:PimplClass)* impl() const; \
PPCAT(implementation:,:PimplClass)* impl();

/*
* PIMPL_ACCESS_IMPLEMENTATION generates the following definition based on MainClass and PimplClass names
*
namespace implementation
{
const PimplClass& PimplClassGet::impl(const MainClass& value)
{
return value.impl();
}
PimplClass& PimplClassGet::impl(MainClass& value)
{
return value.impl();
}
}
const implementation::PimplClass& MainClass::impl() const
{
return *m_impl;
}
implementation::PimplClass& MainClass::impl()
{
return *m_impl;
}
*/
#define PIMPL_ACCESS_IMPLEMENTATION(MainClass, PimplClass) \
namespace implementation \
{ \
    const PimplClass & PPCAT(PimplClass,Get)::impl(const MainClass& value) \
    { \
        return value.impl(); \
    } \
    PimplClass & PPCAT(PimplClass,Get)::impl(MainClass& value) \
    { \
        return value.impl(); \
    } \
} \
const implementation::PimplClass & PPCAT(MainClass:,:impl)() const \
{ \
    return *m_impl; \
} \
implementation::PimplClass & PPCAT(MainClass:,:impl)() \
{ \
return *m_impl; \
}

#define PIMPL_ACCESS_IMPLEMENTATION_BUFFER(MainClass, PimplClass) \
namespace implementation \
{ \
    const PimplClass* PPCAT(PimplClass,Get)::impl(const MainClass& value) \
    { \
        return dynamic_cast<const PimplClass*>(value.impl()); \
    } \
    PimplClass* PPCAT(PimplClass,Get)::impl(MainClass& value) \
    { \
        return dynamic_cast<PimplClass*>(value.impl()); \
    } \
} \
const implementation::PimplClass* PPCAT(MainClass:,:impl)() const \
{ \
    return dynamic_cast<const PimplClass*>(m_impl.get()); \
} \
implementation::PimplClass* PPCAT(MainClass:,:impl)() \
{ \
    return dynamic_cast<PimplClass*>(m_impl.get()); \
}
