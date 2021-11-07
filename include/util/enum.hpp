#ifndef _UTIL_ENUM_HPP
#define _UTIL_ENUM_HPP

template <typename E>
constexpr typename std::underlying_type<E>::type enumval(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

#endif
