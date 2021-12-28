#ifndef _UTIL_ENUM_HPP
#define _UTIL_ENUM_HPP

#include <type_traits>

// Get the underlying value associated to an enum class value
// e.g : enum class MyEnum : int {Hello = 1, World = 15};
// MyEnum message = MyEnum::World;  int value = enumval(message)  // -> value = 15
template <typename E>
constexpr typename std::underlying_type<E>::type enumval(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

#endif
