/* -----------------------------------
 * HTMATCH
 * bittools.h
 * -----------------------------------
 * Defines platform independent tools for dealing with bitwise operations
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
#ifndef _HTMATCH_BIT_TOOLS_H
#define _HTMATCH_BIT_TOOLS_H

#include "system.h"

// Some bitwise macros
//

#define HTMATCH_mask(uBitCount)             ((1u << (uBitCount))-1u)
#define HTMATCH_mask64b(uBitCount)          ((1uLL << (uBitCount))-1uLL)

#if defined (_MSC_VER)
#  include "intrin.h"
#endif

namespace HTMATCH {

    // Fallback software implementations of some bit operations (which otherwise could be solved by some single CPU instruction)
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
#if defined(HTMATCH_FAST64b)
        uint64 uMagic = uValue - ((uValue >> 1u) & 0x5555555555555555uLL);  // now uMagic holds count of set bits in 32 buckets,
                                                                            //   one for each pair of bits
        uMagic = (uMagic & 0x3333333333333333uLL) + ((uMagic >> 2u) & 0x3333333333333333uLL);  // ... see above comments for 32b
        uMagic = (uMagic + (uMagic >> 4u)) & 0x0F0F0F0F0F0F0F0FuLL;
        return (uMagic * 0x0101010101010101uLL) >> 56u;
#else
        // falback to piecewise on 32b if not a 64b target
        uint32 uSum = countSetBits32_software(uint32(uValue)) + countSetBits32_software(uint32(uValue>>32u));
        return uint64(uSum);
#endif
    }

    // Software implementation for getMostSignificantBitPos32, see below
    inline uint32 getMostSignificantBitPos32_software(uint32 uValue) {
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

    // Software implementation for getTrailingZeroesCount32, see below
    inline uint32 getTrailingZeroesCount32_software(uint32 uValue) {
        static const uint32 Lookup[32] = {  0u, 1u, 28u, 2u, 29u, 14u, 24u, 3u, 30u, 22u, 20u, 15u, 25u, 17u, 4u, 8u, 
                                            31u, 27u, 13u, 23u, 21u, 19u, 16u, 7u, 26u, 12u, 18u, 6u, 11u, 5u, 10u, 9u, };
        uint32 uMagic = uValue & uint32(-int32(uValue));    // same kind of trick as 'getMostSignificantBitPos32_software' above
        return Lookup[uint32(uMagic * 0x077CB531u) >> 27u]; // @see https://graphics.stanford.edu/~seander/bithacks.html
        // @see "Using de Bruijn Sequences to Index 1 in a Computer Word"
        //    by Charles E. Leiserson, Harald Prokof, and Keith H. Randall.
    }

    // Software implementation for getTrailingZeroesCount64, see below
    FORCE_INLINE uint32 getTrailingZeroesCount64_software(uint64 uValue) FORCE_INLINE_END {
        uint32 uLo = uint32(uValue);
        if (uLo)
            return getTrailingZeroesCount32_software(uLo);
        else
            return 32u + getTrailingZeroesCount32_software(uint32(uValue>>32u));
    }

    // MSVC and GCC compatible hardware-enhanced bit operations
    // (however fully platform-independent in intent, falling back to a software implementation otherwise)
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
#if defined(HTMATCH_ARCH_x64) || defined(HTMATCH_ARCH_ARM64)
#  if defined (_MSC_VER)
        return __popcnt64(uValue);
#  elif defined(__GNUC__) || defined(__clang__)
        return uint64(__builtin_popcountll(uValue));
#  else
        return countSetBits64_software(uValue);
#  endif
#else
        uint32 uSum = countSetBits32(uint32(uValue)) + countSetBits32(uint32(uValue>>32u));
        return uint64(uSum);
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
#if defined(HTMATCH_ARCH_x64) || defined(HTMATCH_ARCH_ARM64)
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
        uint32 uHi = uint32(uValue>>32u);
        if (uHi)
            return 32u + getMostSignificantBitPos32(uHi);
        else
            return getMostSignificantBitPos32(uint32(uValue));
#endif
    }

    // Counts the number of trailing zeroes in an uint32 value.
    //   Equivalently, returns position of least significant set bit
    //   returns in [0..31] : the position of the least significant bit which was set in the parameter's value.
    //   example: getTrailingZeroesCount(9u) returns 0
    // Warning : if param is zero, result is undefined
    FORCE_INLINE uint32 getTrailingZeroesCount32(uint32 uValue) FORCE_INLINE_END {
#if defined (_MSC_VER)
        unsigned long result;
        _BitScanForward(&result, uValue);
        return result;
#elif defined(__GNUC__) || defined(__clang__)
        int iCountTrailingZeroes = __builtin_ctz(uValue);
        return uint32(iCountTrailingZeroes);
#else
        return getTrailingZeroesCount32_software(uValue);
#endif
    }

    // Counts the number of trailing zeroes in an uint64 value.
    //   Equivalently, returns position of least significant set bit
    //   returns in [0..63] : the position of the least significant bit which was set in the parameter's value.
    //   example: getTrailingZeroesCount(9uLL) returns 0
    // Warning : if param is zero, result is undefined
    FORCE_INLINE uint32 getTrailingZeroesCount64(uint64 uValue) FORCE_INLINE_END {
#if defined(HTMATCH_ARCH_x64) || defined(HTMATCH_ARCH_ARM64)
#  if defined (_MSC_VER)
        unsigned long result;
        _BitScanForward64(&result, uValue);
        return result;
#  elif defined(__GNUC__) || defined(__clang__)
        int iCountTrailingZeroes = __builtin_ctzll(uValue);
        return uint32(iCountTrailingZeroes);
#  else
        return getTrailingZeroesCount64_software(uValue);
#  endif
#else
        uint32 uLo = uint32(uValue);
        if (uLo)
            return getTrailingZeroesCount32(uLo);
        else
            return 32u + getTrailingZeroesCount32(uint32(uValue>>32u));
#endif
    }

    // non-macro version of HTMATCH_mask
    FORCE_INLINE constexpr u32fast mask(u8fast uBitCount) FORCE_INLINE_END {
        return (u32fast(1u) << uBitCount) - u32fast(1u);
    }

    // non-macro version of HTMATCH_mask64b
    FORCE_INLINE constexpr uint64 mask64(u8fast uBitCount) FORCE_INLINE_END {
        return (1uLL << uBitCount) - 1uLL;
    }

    // a few unbranching conditional functions
    //

    // returns the absolute value of a signed 32bits integer in an unbranching fashion.
    FORCE_INLINE constexpr int32 unbranchingAbs(int32 iValue) FORCE_INLINE_END {
        // signed rsh to flood the whole register with sign-bit of the parameter
        int32 iMaskIfNeg = iValue >> 31;
        return ((~iMaskIfNeg) & iValue) | (iMaskIfNeg & (-iValue));
    }

    // returns same signed 32bits integer, however forced to 0 if negative, in an unbranching fashion.
    FORCE_INLINE constexpr int32 unbranchingClampLowToZero(int32 iValue) FORCE_INLINE_END {
        // signed rsh to flood the whole register with sign-bit of the parameter, and bitwise-neg for opposite
        int32 iMaskIfNonNeg = ~(iValue >> 31);
        return iMaskIfNonNeg & iValue;
    }

    // returns same signed 32bits integer, however forced to min value if below, in an unbranching fashion.
    FORCE_INLINE constexpr int32 unbranchingClampLowTo(int32 iMin, int32 iValue) FORCE_INLINE_END {
        // signed rsh to flood the whole register with sign-bit of the subtraction
        int32 iMaskIfLowerThanMin = (iValue - iMin) >> 31;
        return (iMaskIfLowerThanMin & iMin) | ((~iMaskIfLowerThanMin) & iValue);
    }

    // returns same signed 32bits integer, however forced to max value if above, in an unbranching fashion.
    FORCE_INLINE constexpr int32 unbranchingClampHighTo(int32 iMax, int32 iValue) FORCE_INLINE_END {
        // signed rsh to flood the whole register with sign-bit of the subtraction
        int32 iMaskIfHigherThanMax = (iMax - iValue) >> 31;
        return (iMaskIfHigherThanMax & iMax) | ((~iMaskIfHigherThanMax) & iValue);
    }

    // returns shortest distance modulo 2^n in either direction, between two coordinates on 16 bits, in an unbranching fashion.
    //   'uSpanBits' shall represent the actual 'n' of that 2^n above, and 'uMaskSpan' shall equal HTMATCH_mask(n).
    // note that n must be strictly lower than 16, and indices shall be in [0 .. 2^n - 1] themselves
    FORCE_INLINE constexpr u16fast wrappedDistanceBetween(u16fast uIndex1, u16fast uIndex2, u16fast uMaskSpan,
            u16fast uSpanBits) FORCE_INLINE_END {
        // performs diff unsigned
        u16fast uDiff = uIndex1 - uIndex2;
        // masking uDiff against span would give us, in effect quite nicely, an exact and always-positive distance
        //   within 0..2^n-1 already... simply not always the 'shortest'.
        u16fast uAbsDist = uDiff & uMaskSpan;
        // for that, we'd need to take its modulo-opposite (that is, 2^n - dist) whenever it is greater than half.
        u16fast uAdditionalBitsToSign = sizeof(u16fast)*CHAR_BIT - uSpanBits;
        // make it just so that highest bit of the following value is set when that dist above is eq-or-more-than-half
        i16fast iAsSignedShifted = i16fast(uDiff << uAdditionalBitsToSign);
        // signed rsh to flood the whole register with that bit in question above
        u16fast uMaskIfNeedsOpposite = u16fast(iAsSignedShifted >> (sizeof(u16fast)*CHAR_BIT - 1));
        // the 'opposite' in that context is the distance in the reverse direction: 2^n - abs distance
        u16fast uOppositeDist = (u16fast(1u) << uSpanBits) - uAbsDist;
        return ((~uMaskIfNeedsOpposite) & uAbsDist) | (uMaskIfNeedsOpposite & uOppositeDist);
    }

    // returns a power-of-two 'coarser' count from an unsigned 32b value, in an unbranching fashion.
    //   this is equivalent to a 'ceiled' div by 2^n (instead of the floored div result given by "value >> n")
    FORCE_INLINE constexpr uint32 reqCountCoarseTo(uint32 uCount, u8fast uCoarsificationPow2) FORCE_INLINE_END {
        uint32 uLowerBound = uCount >> uCoarsificationPow2; // floored div result. We need that +1 if was not exactly divisible
        const uint32 uMaskRemainder = HTMATCH_mask(uCoarsificationPow2);
        int32 iNegativeIffThereIsARemainder = 0 - int32(uCount & uMaskRemainder);
        // unsigned rsh to result in just '1' iff sign-bit of the subtraction from zero was set
        uint32 uOneMoreIfNeeded = uint32(iNegativeIffThereIsARemainder) >> (sizeof(uint32)*CHAR_BIT - 1u);
        return uLowerBound + uOneMoreIfNeeded;
    }

    // returns a power-of-two 'coarser' count from an unsigned 64b value, in an unbranching fashion.
    //   this is equivalent to a 'ceiled' div by 2^n (instead of the floored div result given by "value >> n")
    FORCE_INLINE constexpr uint64 reqCountCoarse64To(uint64 uCount, u8fast uCoarsificationPow2) FORCE_INLINE_END {
        uint64 uLowerBound = uCount >> uCoarsificationPow2; // floored div result. We need that +1 if was not exactly divisible
        const uint64 uMaskRemainder = HTMATCH_mask64b(uCoarsificationPow2);
        int64 iNegativeIffThereIsARemainder = 0LL - int64(uCount & uMaskRemainder);
        // unsigned rsh to result in just '1' iff sign-bit of the subtraction from zero was set
        uint64 uOneMoreIfNeeded = uint64(iNegativeIffThereIsARemainder) >> (sizeof(uint64)*CHAR_BIT - 1u);
        return uLowerBound + uOneMoreIfNeeded;
    }

    // packed-coordinates expansion macros
    //

    // transforms a tight-packed XY in a single 32b integer to an expanded XY (still in a single 32b integer),
    //   according to the pack/expand template parameters **if it can be performed in a single mul-and-mask**
    //   (otherwise static assert would fire)
    // eg: expandXY<3u,3u,0u,16u> would convert a packed value of the binary form (lsb shown to the right):
    //    0000 0000 0000 0000 0000 0000 00yy yxxx
    // to a result of the binary form:
    //    0000 0000 0000 0yyy 0000 0000 0000 0xxx
    // t-param uBitX: number of bits taken by the X coordinate (from 0 to uBitX-1 in packed)
    // t-param uBitY: number of bits taken by the Y coordinate (from uBitX to uBitX+uBitY-1 in packed)
    // t-param uPosX: desired bit-position of the start of the X coordinate in expanded result
    // t-param uPosY: desired bit-position of the start of the Y coordinate in expanded result
    template<uint8 uBitX, uint8 uBitY, uint8 uPosX, uint8 uPosY>
    FORCE_INLINE const uint32 expandXY(uint32 uPackedValue) FORCE_INLINE_END {
        static_assert(uBitX > 0 && uBitX < sizeof(uint32)*CHAR_BIT, "invalid 32b range for X");
        static_assert(uBitY > 0 && uBitY < sizeof(uint32)*CHAR_BIT, "invalid 32b range for Y");
        static_assert(uPosY <= sizeof(uint32)*CHAR_BIT - uBitY,     "result overflow");
        static_assert(uPosY > uPosX,                                "invalid XY ordering");
        static_assert(uPosX+uBitX+uBitX+1 <= uPosY,                 "intermediate overflow to final Y position");
        static const uint32 uMultiplier = (1u << uPosX) | (1u << (uPosY-uBitX));
        static const uint32 uMask = (uint32(mask(uBitX)) << uPosX) | (uint32(mask(uBitY)) << uPosY);
        return (uPackedValue * uMultiplier) & uMask;
    }
    ; // template termination

    // transforms a tight-packed XYZ in a single 32b integer to an expanded XYZ (still in a single 32b integer),
    //   according to the pack/expand template parameters **if it can be performed in a single mul-and-mask**
    //   (otherwise static assert would fire)
    // eg: expandXYZ<3u,3u,2u,0u,13u,26u> would convert a packed value of the binary form (lsb shown to the right):
    //    0000 0000 0000 0000 0000 0000 zzyy yxxx
    // to a result of the binary form:
    //    0000 zz00 0000 0000 yyy0 0000 0000 0xxx
    // t-param uBitX: number of bits taken by the X coordinate (from 0 to uBitX-1 in packed)
    // t-param uBitY: number of bits taken by the Y coordinate (from uBitX to uBitX+uBitY-1 in packed)
    // t-param uBitZ: number of bits taken by the Z coordinate (from uBitX+uBitY to uBitX+uBitY+uBitZ-1 in packed)
    // t-param uPosX: desired bit-position of the start of the X coordinate in expanded result
    // t-param uPosY: desired bit-position of the start of the Y coordinate in expanded result
    // t-param uPosZ: desired bit-position of the start of the Z coordinate in expanded result
    template<uint8 uBitX, uint8 uBitY, uint8 uBitZ, uint8 uPosX, uint8 uPosY, uint8 uPosZ>
    FORCE_INLINE const uint32 expandXYZ(uint32 uPackedValue) FORCE_INLINE_END {
        static_assert(uBitX > 0 && uBitX < sizeof(uint32)*CHAR_BIT, "invalid 32b range for X");
        static_assert(uBitY > 0 && uBitY < sizeof(uint32)*CHAR_BIT, "invalid 32b range for Y");
        static_assert(uBitZ > 0 && uBitZ < sizeof(uint32)*CHAR_BIT, "invalid 32b range for Z");
        static_assert(uPosZ <= sizeof(uint32)*CHAR_BIT - uBitZ,     "result overflow");
        static_assert(uPosZ > uPosY && uPosY > uPosX,               "invalid XYZ ordering");
        static_assert(uPosX+uBitX+uBitX+1 <= uPosY,                 "intermediate overflow to final Y position");
        static_assert(uPosY+uBitY+uBitX+uBitY+1 <= uPosZ,           "intermediate overflow to final Z position");
        static const uint32 uMultiplier = (1u << uPosX) | (1u << (uPosY-uBitX)) | (1u << (uPosZ-uBitX-uBitY));
        static const uint32 uMask =
            (uint32(mask(uBitX)) << uPosX) | (uint32(mask(uBitY)) << uPosY) | (uint32(mask(uBitZ)) << uPosZ);
        return (uPackedValue * uMultiplier) & uMask;
    }
    ; // template termination

    // 64b version of expandXY, see above
    template<uint8 uBitX, uint8 uBitY, uint8 uPosX, uint8 uPosY>
    FORCE_INLINE const uint64 expandXY64(uint64 uPackedValue) FORCE_INLINE_END {
        static_assert(uBitX > 0 && uBitX < sizeof(uint64)*CHAR_BIT, "invalid 64b range for X");
        static_assert(uBitY > 0 && uBitY < sizeof(uint64)*CHAR_BIT, "invalid 64b range for Y");
        static_assert(uPosY <= sizeof(uint32)*CHAR_BIT - uBitY,     "result overflow");
        static_assert(uPosY > uPosX,                                "invalid XY ordering");
        static_assert(uPosX+uBitX+uBitX+1 <= uPosY,                 "intermediate overflow to final Y position");
        static const uint64 uMultiplier = (1uLL << uPosX) | (1uLL << (uPosY-uBitX));
        static const uint64 uMask = (mask64(uBitX) << uPosX) | (mask64(uBitY) << uPosY);
        return (uPackedValue * uMultiplier) & uMask;
    }
    ; // template termination

    // 64b version of expandXYZ, see above
    template<uint8 uBitX, uint8 uBitY, uint8 uBitZ, uint8 uPosX, uint8 uPosY, uint8 uPosZ>
    FORCE_INLINE const uint64 expandXYZ64(uint64 uPackedValue) FORCE_INLINE_END {
        static_assert(uBitX > 0 && uBitX < sizeof(uint64)*CHAR_BIT, "invalid 64b range for X");
        static_assert(uBitY > 0 && uBitY < sizeof(uint64)*CHAR_BIT, "invalid 64b range for Y");
        static_assert(uBitZ > 0 && uBitZ < sizeof(uint64)*CHAR_BIT, "invalid 64b range for Z");
        static_assert(uPosZ <= sizeof(uint64)*CHAR_BIT - uBitZ,     "result overflow");
        static_assert(uPosZ > uPosY && uPosY > uPosX,               "invalid XYZ ordering");
        static_assert(uPosX+uBitX+uBitX+1 <= uPosY,                 "intermediate overflow to final Y position");
        static_assert(uPosY+uBitY+uBitX+uBitY+1 <= uPosZ,           "intermediate overflow to final Z position");
        static const uint64 uMultiplier = (1uLL << uPosX) | (1uLL << (uPosY-uBitX)) | (1uLL << (uPosZ-uBitX-uBitY));
        static const uint64 uMask = (mask64(uBitX) << uPosX) | (mask64(uBitY) << uPosY) | (mask64(uBitZ) << uPosZ);
        return (uPackedValue * uMultiplier) & uMask;
    }
    ; // template termination

} // namespace HTMATCH


#endif // _HTMATCH_BIT_TOOLS_H

