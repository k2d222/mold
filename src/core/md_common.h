#ifndef _MD_COMMON_H_
#define _MD_COMMON_H_

#include "md_compiler.h"

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#if MD_COMPILER_MSVC
#define STATIC_ASSERT static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif
#endif

#if MD_COMPILER_MSVC
#ifndef THREAD_LOCAL
#define THREAD_LOCAL __declspec(thread)
#endif
#else
#ifndef THREAD_LOCAL
#define THREAD_LOCAL __thread
#endif
#endif

#if MD_COMPILER_MSVC
#ifndef ALIGNAS
#define ALIGNAS(x) __declspec(align(x))
#endif
#else
#ifndef ALIGNAS
#define ALIGNAS(x) __attribute__ ((aligned (x)))
#endif
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define IS_ALIGNED(ptr, alignment) (((uintptr_t)ptr % alignment) == 0)
#define NEXT_ALIGNED_ADRESS(ptr, alignment) ((char*)ptr + alignment - ((uint64_t)ptr & (alignment - 1)))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) > 0 ? (x) : -(x))
#define CLAMP(val, _min, _max) MIN(MAX(val, _min), _max)

// Round up division
#define DIV_UP(x, y) ((x + (y-1)) / y)

// Rounds up x to nearest multiple of y
#define ROUND_UP(x, y) (y * DIV_UP(x,y))

#define DEG_TO_RAD(x) ((x)*(3.1415926535897932 / 180.0))
#define RAD_TO_DEG(x) ((x)*(180.0 / 3.1415926535897932))

#define KILOBYTES(x) (x * 1024LL)
#define MEGABYTES(x) (x * 1024LL * 1024LL)

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

#ifdef  __cplusplus

struct ExitScopeHelp {
    template <typename T>
    struct ExitScope {
        T lambda;
        ExitScope(T lambda) : lambda(lambda) {}
        ~ExitScope() { lambda(); }
        ExitScope& operator=(const ExitScope&) = delete;
    };

    template <typename T>
    ExitScope<T> operator+(T t) {
        return t;
    }
};

#define defer [[maybe_unused]] const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

#endif //  __cplusplus

#endif