/* -----------------------------------
 * HTMATCH
 * rand.h
 * -----------------------------------
 * Defines a platform-independent random number generator
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
#ifndef _HTMATCH_RAND_H
#define _HTMATCH_RAND_H

#include "system.h"

namespace HTMATCH {

// - - - - - - - - - - - - - - - -
// Rand
//   a class handling 32b random number generation (and convenience wrappers to float, etc.)
//   User shall initialize one instance, then draw a new number by calling getNext()
//     (or one of the convenience wrappers around it) repeatedly.
//   Initializing two instances with same seed (or re-seeding to an original seed) ensures same deterministic 'random' sequence
//     (on all platforms)
// - - - - - - - - - - - - - - - -
// Based on a 'KISS' Random number generator, among algorithms recommended by George Marsaglia.
// Repeat period thought to be on the order of 2^123, with very good randomness properties for any usage.
//   Good performance. NOT cryptographically secure, however.
// - - - - - - - - - - - - - - - -
class Rand
{
public:

    static const uint32 k_DefaultX = 123456789u;
    static const uint32 k_DefaultY = 987654321u;
    static const uint32 k_DefaultZ =  43219876u;
    static const uint32 k_DefaultC =   6543217u;

    // Defines an instance of 'Rand', initialized with the specified seed.
    // Do NOT set uValueY to 0 ; also avoid uValueZ = uValueC = 0
    explicit Rand(uint32 uValueX = k_DefaultX, uint32 uValueY = k_DefaultY,
        uint32 uValueZ = k_DefaultZ, uint32 uValueC = k_DefaultC) {
        seed(uValueX, uValueY, uValueZ, uValueC);
    }

    // Initializes the class using the specified seed.
    // Do NOT set uValueY to 0 ; also avoid uValueZ = uValueC = 0
    void seed(uint32 uValueX = k_DefaultX, uint32 uValueY = k_DefaultY,
        uint32 uValueZ = k_DefaultZ, uint32 uValueC = k_DefaultC) {
        _uX = uValueX;
        _uY = uValueY;
        _uZ = uValueZ;
        _uC = uValueC;
    }

    // Returns next number from this generator. Probability of any value shall be very close to uniform in [0 .. 4 294 967 295]
    uint32 getNext() {
        _uX = (314527869 * _uX) + 1234567;
        _uY ^= (_uY << 5); _uY ^= (_uY >> 7); _uY ^= (_uY << 22);
        uint64 t = (4294584393uLL * _uZ) + _uC;
        _uC = uint32(t>>32);
        _uZ = uint32(t);
        return _uX + _uY + _uZ;
    }

    FORCE_INLINE static constexpr uint32 _fromZeroToExcl(uint32 uOverMax, uint32 uDraw) {
        return uOverMax % uDraw;
    }
    FORCE_INLINE static constexpr double _asDouble01(uint32 uDraw) {
        return double(uDraw) * (1.0 / 4294967296.0);
    }
    FORCE_INLINE static constexpr double _asDouble01inclusive(uint32 uDraw) {
        return double(uDraw) / 4294967295.0;
    }
    FORCE_INLINE static constexpr double _asDoubleNeg1Pos1(uint32 uDraw) {
        return 1.0 - (double(uDraw) / 2147483647.5);
    }

    // Returns next number from this generator, hopefully uniform in [0 .. uOverMax-1]
    //   this version is fast, however not *precisely* uniform even if getNext() is, although not significantly so for
    //   relatively small 'uOverMax'
    FORCE_INLINE uint32 drawNextFromZeroToExcl(uint32 uOverMax) FORCE_INLINE_END {
        return Rand::_fromZeroToExcl(uOverMax, getNext());
    }

    // Returns next number from this generator, hopefully uniform in [0 .. uOverMax-1]
    //   this version will enforce uniformity of result (provided getNext() is uniform itself), by re-drawing until so
    //   => please refrain from using very, very high (above 2 billions, for example) values for 'uOverMax'
    uint32 drawNextFromZeroToExcl_forceUniform(uint32 uOverMax) {
        uint32 uNext = getNext();
        const uint32 uRemainderWhenhigh = 0xFFFFFFFFu % uOverMax;
        if (uRemainderWhenhigh < uOverMax-1u) {    // if result when max uint is less than max remainder, then whenever
            uint32 uMax = 0xFFFFFFFFu - uRemainderWhenhigh; // we draw one of the very-highest results (above max uint minus
            while (uNext >= uMax)                           // that value), we're not sampling the range uniformly
                uNext = getNext();                          // => force drawing a brand new value, until we are
        }
        return uNext % uOverMax;
    }

    // Returns next number from this generator, hopefully uniform in [0.0 .. 1.0)
    FORCE_INLINE double getNextAsDouble01() FORCE_INLINE_END {
        return Rand::_asDouble01(getNext());
    }
    // Returns next number from this generator, hopefully uniform in [0.0 .. 1.0]
    FORCE_INLINE double getNextAsDouble01inclusive() FORCE_INLINE_END {
        return Rand::_asDouble01inclusive(getNext());
    }
    // Returns next number from this generator, hopefully uniform in [-1.0 .. 1.0]
    FORCE_INLINE double getNextAsDoubleNeg1Pos1() FORCE_INLINE_END {
        return Rand::_asDoubleNeg1Pos1(getNext());
    }

    // Returns next number from this generator as 32b floating point, hopefully uniform in [0.0 .. 1.0)
    FORCE_INLINE float getNextAsFloat01() FORCE_INLINE_END {
        return float(Rand::_asDouble01(getNext()));
    }
    // Returns next number from this generator as 32b floating point, hopefully uniform in [0.0 .. 1.0]
    FORCE_INLINE float getNextAsFloat01inclusive() FORCE_INLINE_END {
        return float(Rand::_asDouble01inclusive(getNext()));
    }
    // Returns next number from this generator as 32b floating point, hopefully uniform in [-1.0 .. 1.0]
    FORCE_INLINE float getNextAsFloatNeg1Pos1() FORCE_INLINE_END {
        return float(Rand::_asDoubleNeg1Pos1(getNext()));
    }

private:
    uint32 _uX;
    uint32 _uY;
    uint32 _uZ;
    uint32 _uC;
};

} // namespace HTMATCH

#endif // _HTMATCH_RAND_H

