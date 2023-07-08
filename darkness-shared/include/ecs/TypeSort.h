#pragma once

#include "ComponentTypeStorage.h"
#include <type_traits>
#include <string_view>

namespace ecs
{
    template <typename T>
    constexpr auto type_name() noexcept {
        std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef __clang__
        name = __PRETTY_FUNCTION__;
        prefix = "auto type_name() [T = ";
        suffix = "]";
#elif defined(__GNUC__)
        name = __PRETTY_FUNCTION__;
        prefix = "constexpr auto type_name() [with T = ";
        suffix = "]";
#elif defined(_MSC_VER)
        name = __FUNCSIG__;
        prefix = "auto __cdecl type_name<";
        suffix = ">(void) noexcept";
#else
        static_assert(false, "Unsupported compiler!");
#endif
        name.remove_prefix(prefix.size());
        name.remove_suffix(suffix.size());
        return name;
    }

    template <class... Ts>
    struct list;

    template <template <class...> class Ins, class...> struct instantiate;

    template <template <class...> class Ins, class... Ts>
    struct instantiate<Ins, list<Ts...>> {
        using type = Ins<Ts...>;
    };

    template <template <class...> class Ins, class... Ts>
    using instantiate_t = typename instantiate<Ins, Ts...>::type;


    template <class...> struct concat;

    template <class... Ts, class... Us>
    struct concat<list<Ts...>, list<Us...>>
    {
        using type = list<Ts..., Us...>;
    };

    template <class... Ts>
    using concat_t = typename concat<Ts...>::type;

    template <int Count, class... Ts>
    struct take;

    template <int Count, class... Ts>
    using take_t = typename take<Count, Ts...>::type;

    template <class... Ts>
    struct take<0, list<Ts...>> {
        using type = list<>;
        using rest = list<Ts...>;
    };

    template <class A, class... Ts>
    struct take<1, list<A, Ts...>> {
        using type = list<A>;
        using rest = list<Ts...>;
    };

    template <int Count, class A, class... Ts>
    struct take<Count, list<A, Ts...>> {
        using type = concat_t<list<A>, take_t<Count - 1, list<Ts...>>>;
        using rest = typename take<Count - 1, list<Ts...>>::rest;
    };

    template <class... Types>
    struct sort_list;

    template <class... Ts>
    using sorted_list_t = typename sort_list<Ts...>::type;

    template <class A>
    struct sort_list<list<A>> {
        using type = list<A>;
    };

    template <class Left, class Right>
    static constexpr bool less_than = type_name<Left>() < type_name<Right>();

    template <class A, class B>
    struct sort_list<list<A, B>> {
        using type = std::conditional_t<less_than<A, B>, list<A, B>, list<B, A>>;
    };

    template <class...>
    struct merge;

    template <class... Ts>
    using merge_t = typename merge<Ts...>::type;

    template <class... Bs>
    struct merge<list<>, list<Bs...>> {
        using type = list<Bs...>;
    };

    template <class... As>
    struct merge<list<As...>, list<>> {
        using type = list<As...>;
    };

    template <class AHead, class... As, class BHead, class... Bs>
    struct merge<list<AHead, As...>, list<BHead, Bs...>> {
        using type = std::conditional_t<less_than<AHead, BHead>,
            concat_t<list<AHead>, merge_t<list<As...>, list<BHead, Bs...>>>,
            concat_t<list<BHead>, merge_t<list<AHead, As...>, list<Bs...>>>
        >;
    };

    template <class... Types>
    struct sort_list<list<Types...>> {
        static constexpr auto first_size = sizeof...(Types) / 2;
        using split = take<first_size, list<Types...>>;
        using type = merge_t<
            sorted_list_t<typename split::type>,
            sorted_list_t<typename split::rest>>;
    };

    template <class... Ts>
    struct ActualSortedTypeContainer {};

    template <class... Ts>
    using SortedTypeContainer = instantiate_t<ActualSortedTypeContainer, sorted_list_t<list<Ts...>>>;

}
