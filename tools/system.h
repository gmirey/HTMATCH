/* -----------------------------------
 * HTMATCH
 * system.h
 * -----------------------------------
 * Defines platform independent types and tools
 * Note: HTMATCH is currently only "optimized" for MSVC or GCC compilers (clang should be GCC equiv)
 *   also, it will *by far* prefer x64 targets.
 *
 * Copyright 2019, Guillaume Mirey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HTMATCH_SYSTEM_H
#define _HTMATCH_SYSTEM_H

// Trying to detect target arch from common compiler-defined macros
//

#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__)
#  define HTMATCH_ARCH_x64
#elif defined(_M_ARM64) || (defined(__aarch64__)
#  define HTMATCH_ARCH_ARM64
#endif

#if defined(HTMATCH_ARCH_x64) || defined(HTMATCH_ARCH_ARM64)
#  define HTMATCH_FAST64b
#endif

// MSVC and GCC compatible force-inline macros
//

#if defined (_MSC_VER)
#  define FORCE_INLINE            __forceinline
#  define FORCE_INLINE_END
#else
#  define FORCE_INLINE            inline
#  define FORCE_INLINE_END        __attribute__((always_inline))
#endif

#include "stdint.h"
#include "stddef.h"
#include "limits.h"
#if defined (_MSC_VER)
#  include "intrin.h"
#endif

namespace HTMATCH {

    // Fixed size integers
    // Note: we keep uint8 and int8 defined as variants of 'char' to be less uncertain about some hairy aliasing concerns
    // But we'll also make sure 'char' is exactly 8b indeed...
#if (CHAR_BIT != 8)
#  error "uncommon architecture detected - HTMATCH cannot recover"
#endif
    // To the contrary...
    // We're unsure why the choice of 'char' for the 'int_fast8_t' definition in, eg., MSVC library... in the event that it stemmed
    //   from same aliasing concerns as expressed above, then the following macro definition will force it to use next-size-fastest,
    //   that is, int_fast16_t, which for MSVC library turns out to be 32b (and seems to make more sense for most known processors)
    // YMMV here... => you can experiment with disabling this macro at will...
    #define HTMATCH_COERCE_FAST8_TO_FAST16

    // Fixed-size integers

    typedef unsigned char           uint8;
    typedef uint16_t                uint16;
    typedef uint32_t                uint32;
    typedef uint64_t                uint64;

    typedef char                    int8;
    typedef int16_t                 int16;
    typedef int32_t                 int32;
    typedef int64_t                 int64;

    // Fastest sizes integers, hopefully for target arch... in practice often just 1 per provider of the stdint library :x
    // ... go figure

#ifdef HTMATCH_COERCE_FAST8_TO_FAST16
    typedef uint_fast16_t           u8fast;
    typedef int_fast16_t            i8fast;
#else
    typedef uint_fast8_t            u8fast;
    typedef int_fast8_t             i8fast;
#endif

    typedef uint_fast16_t           u16fast;
    typedef uint_fast32_t           u32fast;

    typedef int_fast16_t            i16fast;
    typedef int_fast32_t            i32fast;

} // namespace HTMATCH

// Other useful macros
//

#define HTMATCH_unused(_param) do { (void)sizeof(_param); } while(0)


// Alignment concerns
//

#if defined(_MSC_VER) // for some reason... *sigh*
#  include "malloc.h"
#  define HTMATCH_aligned_alloc(alignment, size)    ::_aligned_malloc(size, alignment)
#  define HTMATCH_aligned_free(ptr)                 ::_aligned_free(ptr)
#else
#  define HTMATCH_aligned_alloc(alignment, size)    std::aligned_alloc(alignment, size)
#  define HTMATCH_aligned_free(ptr)                 std::free(ptr)
#endif


// type-punning concerns, and macro
// for an explanation of what's at stake here, see "What is Strict Aliasing and Why do we Care?" article
// https://gist.github.com/shafik/848ae25ee209f698763cffee272a58f8
//
// declaring the following macros:
//   HTMATCH_tpunptr(_aliasingType, _aliasingVar, _aliasedType, _aliasedVar, _byteSize)
//   HTMATCH_tpunval(_aliasingType, _aliasingVar, _aliasedType, _aliasedVar)
//

// Disabled: not yet ready or sufficiently tested
// Note: in the meantime, HTMATCH widespread usage of uint8 buffers should be safe regarding those aliasing issues anyway

/*

// You may uncomment 0 or one from the following at will
#define HTMATCH_USE_UNION_FOR_TYPE_PUNNING
//#define HTMATCH_USE_MEMCPY_FOR_TYPE_PUNNING

// You may uncomment this one at will; will force a safe choice if neither of the above was defined yet
//#define HTMATCH_PREFER_SAFE_FOR_TYPE_PUNNING

#if !defined(HTMATCH_USE_UNION_FOR_TYPE_PUNNING) && !defined(HTMATCH_USE_MEMCPY_FOR_TYPE_PUNNING)
#  if defined(__GNUC__)
#    define HTMATCH_USE_UNION_FOR_TYPE_PUNNING
#  elif defined(HTMATCH_PREFER_SAFE_FOR_TYPE_PUNNING)
#    define HTMATCH_USE_MEMCPY_FOR_TYPE_PUNNING
#  endif
#endif

#if   defined(HTMATCH_USE_UNION_FOR_TYPE_PUNNING)

// type-punning macros
// plain C way of going through an union, ensured safe for C++ also by gcc spec. although not safe-by-'C++'-standard
//   clang compiler seems to behave identically to gcc in that regard
//   as of 2019, seems also best-way for MSVC (... or totally broken, need to investigate further)

#  define HTMATCH_tpunptr(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue, _byteSize) do { \
    static_assert(sizeof(_aliasingType) <= alignof(_aliasedValue)); \
    struct { \
        _aliasedType*  _orig; \
        _aliasingType* _aliased; \
    } _aliasingUnion; \
    _aliasingUnion._orig = _aliasedValue; \
    _aliasingValue = _aliasingUnion._aliased; \
} while(0)
#  define HTMATCH_tpunval(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue) do { \
    static_assert(sizeof(_aliasingType) == sizeof(_aliasedType)); \
    struct { \
        _aliasedType  _orig; \
        _aliasingType _aliased; \
    } _aliasingUnion; \
    _aliasingUnion._orig = _aliasedValue; \
    _aliasingValue = _aliasingUnion._aliased; \
} while(0)

#elif defined(HTMATCH_USE_MEMCPY_FOR_TYPE_PUNNING)

// type-punning macros
// safe way using 'memcpy', totally defeating the purpose of a type-punning "optim" in the first place, BUT
//   hopefully detectable and optimizable-away by a good compiler.
//   as of 2019:
//     gcc -O2 is able to emit exact same code with memcpy as with the other two methods, at least for HTMATCH_tpunval
//     MSVC /O2 is able to optimize the call away at least for HTMATCH_tpunval, although still seems bloated.

#  define HTMATCH_tpunptr(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue, _byteSize) do { \
    static_assert(sizeof(_aliasingType) <= alignof(_aliasedValue)); \
    std::memcpy(_aliasingValue, _aliasedType, _byteSize); \
} while(0)

#  define HTMATCH_tpunval(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue) do { \
    static_assert(sizeof(_aliasingType) == sizeof(_aliasedType)); \
    std::memcpy(&_aliasingValue, &_aliasedType, sizeof(_aliasedType)); \
} while(0)

#else

// type-punning macros
//   simple way using pointer aliasing, although not safe-by-standard.
//   plainly understandable in intent, though, and should(?) be safe in debug mode.

#  define HTMATCH_tpunptr(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue, _byteSize) do { \
    static_assert(sizeof(_aliasingType) <= alignof(_aliasedValue)); \
    _aliasingValue = reinterpret_cast<_aliasingType*>(_aliasedValue); \
} while(0)

#  define HTMATCH_tpunval(_aliasingType, _aliasingValue, _aliasedType, _aliasedValue) do { \
    static_assert(sizeof(_aliasingType) == sizeof(_aliasedType)); \
    _aliasingValue = *reinterpret_cast<_aliasingType*>(&(_aliasedValue)); \
} while(0)

#endif

*/

#endif // _HTMATCH_SYSTEM_H

