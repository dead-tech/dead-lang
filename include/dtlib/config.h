#ifndef DTLIB_CONFIG_H
#define DTLIB_CONFIG_H

#ifdef __cpp_constexpr
#define DTLIB_CONSTEXPR constexpr
#else
#define DTLIB_CONSTEXPR
#endif

#if defined(DTLIB_DEBUG_MODE) || !defined(NDEBUG)
#define DTLIB_DEBUG
#endif

#ifndef DTLIB_ABORT
#include <exception>
#define DTLIB_ABORT(...) std::terminate()
#endif

#ifndef DTLIB_ASSERT
#ifdef DTLIB_DEBUG
#define DTLIB_ASSERT(cond) \
    if (!(cond)) { DTLIB_ABORT(); }
#else
#define DTLIB_ASSERT(cond) \
    {                      \
    }
#endif
#endif

// [[nodiscard()]] attribute definition
#ifndef DTLIB_NODISCARD
#if __has_cpp_attribute(nodiscard) >= 201907L
#define DTLIB_NODISCARD(reason) [[nodiscard(reason)]]
#elif __has_cpp_attribute(nodiscard)
#define DTLIB_NODISCARD(reason) [[nodiscard]]
#else
#define DTLIB_NODISCARD()
#endif
#endif

#ifndef DTLIB_DEPRECATED
#if __has_cpp_attribute(deprecated)
#define DTLIB_DEPRECATED(reason) [[deprecated(reason)]]
#else
#define DTLIB_DEPRECATED(reason)
#endif
#endif

#ifndef DTLIB_DEPRECATE_BLOCK
#if defined(DTLIB_ALLOW_DEPRECATED)
#define DTLIB_DEPRECATE_BLOCK(code) code
#else
#define DTLIB_DEPRECATE_BLOCK(code) static_assert(true, "")
#endif
#endif

#endif//DTLIB_CONFIG_H
