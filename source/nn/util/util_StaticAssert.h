#ifndef NN_UTIL_UTIL_STATICASSERT_H_
#define NN_UTIL_UTIL_STATICASSERT_H_

#include <type_traits>

#define NN_STATIC_ASSERT(condition) static_assert(condition, #condition)
#define NN_STATIC_ASSERT_IS_POD(type) static_assert(std::is_trivial<type>::value, #type " is not POD")

#endif
