/* -----------------------------------
 * HTMATCH
 * sdr.h
 * -----------------------------------
 * Currently only defines a simple static class for a quite handy conversion method from a vector of indices to a bitfield
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
#ifndef _HTMATCH_SDR_H
#define _HTMATCH_SDR_H

#include "system.h"      // for uint16, uint64
#include <cstring>       // for memset
#include <vector>        // guess why

namespace HTMATCH {

class SDRTools {
public:

    // - - - - - - - - - - - - - - - - - - - -
    // Converts a vector of indices (a sparse representation of activity) to a binary field where bits positionned at
    //   indices in vector are set to 1, and all other to 0.
    // Binary field is pointed to by a pointer to 64b values, which is assumed large enough for storing those indices.
    //     (max addressable by uint16 indices is 8192 bytes, ie. 1024x 64b values... but the buffer can be smaller iff input
    //    dimension is known smaller and indices ensured to be within those dimensions indeed).
    // uByteCount must be a multiple of 8
    // - - - - - - - - - - - - - - - - - - - -
    static void toBinaryBitmap64(const std::vector<uint16>& vecInputIndices,
        uint64* pOutputBinaryBitmap, const size_t uByteCount) {
        std::memset((void*)pOutputBinaryBitmap, 0, uByteCount);    // ... used to fill that buffer with all zeroes beforehand.
        // then we simply need to parse the input vector and set only those bits corresponding to the provided indices to 1
        for (auto it = vecInputIndices.begin(), itEnd = vecInputIndices.end(); it != itEnd; it++) {
            uint16 uIndex = *it;
            uint16 uQword = uIndex >> 6u;       // index of the 64b value in the buffer is bit index div 64 (64b per QWORD, 2^6)
            uint16 uBit = uIndex & 0x003Fu;     // 3Fu is binary 00111111, hence 6b mask for [0..63] remainder of the above div
            pOutputBinaryBitmap[uQword] |= (1uLL << uBit);        // Sets the corresponding bit to 1
        }
        // that's all, folks!
    }

};

} // namespace HTMATCH

#endif // _HTMATCH_SDR_H

