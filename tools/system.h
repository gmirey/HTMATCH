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

#if defined(_WIN64) || defined(__x86_64__)
#  define HTMATCH_ARCH_x64
#endif

// MSVC and GCC compatible force-inline macros
//

#if defined (_MSC_VER)
#  define FORCE_INLINE            __forceinline
#  define FORCE_INLINE_END
#else
#  define FORCE_INLINE            inline
#  define FORCE_INLINE_END      __attribute__((always_inline))
#endif

#include <cstdint>
#include <cstddef>
#include <climits>
#if defined (_MSC_VER)
#  include "intrin.h"
#endif
namespace HTMATCH {

    // Fixed size integers
    // Note: we keep uint8 and int8 defined as variants of 'char' to be less uncertain about some hairy aliasing concerns
#if (CHAR_BIT != 8)
#  error "uncommon architecture detected - HTMATCH cannot recover"
#endif
    // To the contrary...
    // We're unsure why the choice of 'char' for the 'int_fast8_t' definition in, eg., MSVC library... in the event that it stemmed
    //   from same aliasing concerns as expressed above, then the following macro definition will force it to use next-size-fastest,
    //   that is, int_fast16_t, which for MSVC library turns out to be 32b (and seems to make more sense for most known processors)
    // YMMV here... => you can experiment with disabling this macro at will...
    #define HTMATCH_COERCE_FAST8_TO_FAST16        1

    typedef unsigned char           uint8;
    typedef uint16_t                uint16;
    typedef uint32_t                uint32;
    typedef uint64_t                uint64;

    typedef char                    int8;
    typedef int16_t                 int16;
    typedef int32_t                 int32;
    typedef int64_t                 int64;

#ifdef HTMATCH_COERCE_FAST8_TO_FAST16
    typedef uint_fast16_t           u8fast;
    typedef int_fast16_t            i8fast;
#else
    typedef uint_fast8_t            u8fast;
    typedef int_fast8_t             i8fast;
#endif

    // Fastest sizes integers, hopefully for target arch... in practice often just 1 per provider of the stdint library :x

    typedef uint_fast16_t           u16fast;
    typedef uint_fast32_t           u32fast;

    typedef int_fast16_t            i16fast;
    typedef int_fast32_t            i32fast;

    // Fallback software implementations of some bit operations
    //

    // Software implementation for countSetBits32, see below
    FORCE_INLINE uint32 countSetBits32_software(uint32 uValue) FORCE_INLINE_END {
        uint32 uMagic = uValue - ((uValue >> 1u) & 0x55555555u);            // now uMagic holds count of set bits in 16 buckets,
                                                                            //   one for each pair of bits
        uMagic = (uMagic & 0x33333333u) + ((uMagic >> 2u) & 0x33333333u);   // now uMagic holds count of set bits in 8 buckets,
                                                                            //   one for each quadruplet of bits
        uMagic = (uMagic + (uMagic >> 4u)) & 0x0F0F0F0Fu;                   // now uMagic holds count of set bits in 4 buckets,
                                                                            //   one for each byte
        return (uMagic * 0x01010101u) >> 24u;   // most significant byte after mul contains our result => extracted by the rsh 24
    }

    // Software implementation for countSetBits64, see below
    FORCE_INLINE uint64 countSetBits64_software(uint64 uValue) FORCE_INLINE_END {
#if defined(HTMATCH_ARCH_x64)
        uint64 uMagic = uValue - ((uValue >> 1u) & 0x5555555555555555uLL);  // now uMagic holds count of set bits in 32 buckets,
                                                                            //   one for each pair of bits
        uMagic = (uMagic & 0x3333333333333333uLL) + ((uMagic >> 2u) & 0x3333333333333333uLL);  // ... see above comments for 32b
        uMagic = (uMagic + (uMagic >> 4u)) & 0x0F0F0F0F0F0F0F0FuLL;
        return (uMagic * 0x0101010101010101uLL) >> 56u;
#else
        // falback to piecewise on 32b if not a 64b target
        uint32 uSum = countSetBits32_software(uint32(uValue))) + countSetBits32_software(uint32(uValue>>32u));
        return uint64(uSum);
#endif
    }

    // Software implementation for getMostSignificantBitPos32, see below
    uint32 getMostSignificantBitPos32_software(uint32 uValue) {
        static const uint32 Lookup[32] = {  0u, 9u, 1u, 10u, 13u, 21u, 2u, 29u, 11u, 14u, 16u, 18u, 22u, 25u, 3u, 30u,
                                            8u, 12u, 20u, 28u, 15u, 17u, 24u, 7u, 19u, 27u, 23u, 6u, 26u, 5u, 4u, 31u, };
        uint32 uMagic = uValue | (uValue >> 1u);
        uMagic |= uMagic >> 2u;
        uMagic |= uMagic >> 4u;
        uMagic |= uMagic >> 8u;
        uMagic |= uMagic >> 16u;
        return Lookup[uint32(uMagic * 0x07C4ACDDu) >> 27u];     // Cryptic, heh ?
        // @see https://graphics.stanford.edu/~seander/bithacks.html, credited to Eric Cole and Mark Dickinson.
    }

    // Software implementation for getMostSignificantBitPos64, see below
    FORCE_INLINE uint32 getMostSignificantBitPos64_software(uint64 uValue) FORCE_INLINE_END {
        uint32 uHi = uint32(uValue>>32u);
        if (uHi)
            return 32u + getMostSignificantBitPos32_software(uHi);
        else
            return getMostSignificantBitPos32_software(uint32(uValue));
    }

    // MSVC and GCC compatible hardware-enhanced bit operations
    // (however fully platform-independent in intent, falling back to an arithmetic implementation otherwise)
    //

    // Counts the number of set bits in an uint32 value
    //   returns in [0..32] : the number of bits with value 1 found in the parameter's value
    //   example: countSetBits32(9u) returns 2
    FORCE_INLINE uint32 countSetBits32(uint32 uValue) FORCE_INLINE_END {
#if defined (_MSC_VER)
        return __popcnt(uValue);
#elif defined(__GNUC__) || defined(__clang__)
        return uint32(__builtin_popcount(uValue));
#else
        return countSetBits32_software(uValue);
#endif
    }

    // Counts the number of set bits in an uint64 value
    //   returns in [0..64] : the number of bits with value 1 found in the parameter's value
    //   example: countSetBits64(9uLL) returns 2
    FORCE_INLINE uint64 countSetBits64(uint64 uValue) FORCE_INLINE_END {
#if defined(HTMATCH_ARCH_x64)
#  if defined (_MSC_VER)
        return __popcnt64(uValue);
#  elif defined(__GNUC__) || defined(__clang__)
        return uint64(__builtin_popcountll(uValue));
#  else
        return countSetBits64_software(uValue);
#  endif
#else
        return countSetBits64_software(uValue);
#endif
    }

    // Returns the position of the most significant set bit in an uint32 value.
    //   Equivalently, it is the integer log base 2 (floored) of the value.
    //   returns in [0..31] : the position of the most significant bit which was set in the parameter's value.
    //   example: getMostSignificantBitPos32(9u) returns 3
    // Warning : if param is zero, result is undefined
    FORCE_INLINE uint32 getMostSignificantBitPos32(uint32 uValue) FORCE_INLINE_END {
#if defined (_MSC_VER)
        unsigned long result;
        _BitScanReverse(&result, uValue);
        return result;
#elif defined(__GNUC__) || defined(__clang__)
        int iCountLeadingZeroes = __builtin_clz(uValue);
        return uint32(31 - iCountLeadingZeroes);
#else
        return getMostSignificantBitPos32_software(uValue);
#endif
    }

    // Returns the position of the most significant set bit in an uint64 value.
    //   Equivalently, it is the integer log base 2 (floored) of the value.
    //   returns in [0..63] : the position of the most significant bit which was set in the parameter's value.
    //   example: getMostSignificantBitPos64(9uLL) returns 3
    // Warning : if param is zero, result is undefined
    FORCE_INLINE uint32 getMostSignificantBitPos64(uint64 uValue) FORCE_INLINE_END {
#if defined(HTMATCH_ARCH_x64)
#  if defined (_MSC_VER)
        unsigned long result;
        _BitScanReverse64(&result, uValue);
        return result;
#  elif defined(__GNUC__) || defined(__clang__)
        int iCountLeadingZeroes = __builtin_clzll(uValue);
        return uint32(63 - iCountLeadingZeroes);
#  else
        return getMostSignificantBitPos64_software(uValue);
#  endif
#else
        return getMostSignificantBitPos64_software(uValue);
#endif
    }

    // a few unbranching conditional functions
    //

    FORCE_INLINE int32 unbranchingAbs(int32 iValue) {
        int32 iMaskIfNeg = iValue >> 31;
        return ((~iMaskIfNeg) & iValue) | (iMaskIfNeg & (-iValue));
    }
    FORCE_INLINE int32 unbranchingClampLowToZero(int32 iValue) {
        int32 iMaskIfNonNeg = ~(iValue >> 31);
        return iMaskIfNonNeg & iValue;
    }
    FORCE_INLINE int32 unbranchingClampLowTo(int32 iMin, int32 iValue) {
        int32 iMaskIfLowerThanMin = (iValue - iMin) >> 31;
        return (iMaskIfLowerThanMin & iMin) | ((~iMaskIfLowerThanMin) & iValue);
    }
    FORCE_INLINE int32 unbranchingClampHighTo(int32 iMax, int32 iValue) {
        int32 iMaskIfHigherThanMax = (iMax - iValue) >> 31;
        return (iMaskIfHigherThanMax & iMax) | ((~iMaskIfHigherThanMax) & iValue);
    }
    FORCE_INLINE u16fast wrappedDistanceBetween(u16fast uIndex1, u16fast uIndex2, u16fast uMaskSpan, u16fast uSpanBits) {
        u16fast uDiff = uIndex1 - uIndex2;
        u16fast uAdditionalBitsToSign = sizeof(u16fast)*8u - uSpanBits;
        i16fast iAsSigned = i16fast(uDiff << uAdditionalBitsToSign) >> uAdditionalBitsToSign;
        i16fast iMaskIfNeg = iAsSigned >> (sizeof(u16fast)*8u - 1);
        return u16fast(((~iMaskIfNeg) & iAsSigned) | (iMaskIfNeg & (-iAsSigned)));
    }

} // namespace HTMATCH

// Some bitwise macros
//

#define _mask8b(uBitCount)          uint8((1u << (uBitCount))-1u)
#define _mask16b(uBitCount)         uint16((1u << (uBitCount))-1u)
#define _mask32b(uBitCount)         uint32((1u << (uBitCount))-1u)
#define _mask64b(uBitCount)         uint64((1uLL << (uBitCount))-1uLL)


#endif // _HTMATCH_SYSTEM_H

