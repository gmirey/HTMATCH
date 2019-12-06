/* -----------------------------------
 * HTMATCH
 * mempage.h
 * -----------------------------------
 * Defines a reusable class for dealing with various custom-pagination
 *   schemes used by HTMATCH, using pages with fixed amount of 'slots', each
 *   of a fixed size, to improve allocation speeds.
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

#ifndef _HTMATCH_MEM_PAGE_H
#define _HTMATCH_MEM_PAGE_H

#include "system.h"
#include "bittools.h"
#include <stdexcept>
#include <cstring>
#include <cstdlib>

namespace HTMATCH {

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Base class for a mem page, not 'storing' its own size or count properties, and thus requiring
    //   them as explicit parameters at various call sites. This *may* avoid some unnecessary
    //   fetching of a "header" cache line at the root when we're able to know those values from
    //   other means... but really is not certain and I'm guilty of some preemptive optim here.
    // In any case, this class is wrapped in the more practical 'DefaultMemPage' class below
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    class MemPageBase {
    public:
        MemPageBase(u32fast uSlotCount, u32fast uSlotByteSize, u8fast uAlignBits = 0u):
            _uNextIndex(0u), _uAllocatedCount(0u)
        {
            u32fast uMaxQwords = getMaxQwordsFor(uSlotByteSize, uSlotCount);
            _pAvailabilityField = new uint64[uMaxQwords];
            std::memset((void*)_pAvailabilityField, 0xFF, uMaxQwords * sizeof(uint64));
            if (uAlignBits == 0u) {
                uAlignBits = getMostSignificantBitPos32(uSlotByteSize);
                if (uSlotByteSize & mask(uAlignBits))
                    uAlignBits++;
            }
            _pData = (uint8*)HTMATCH_aligned_alloc(size_t(1u) << uAlignBits, size_t(uSlotByteSize) * size_t(uSlotCount));
        }
        ~MemPageBase() {
            delete[] _pAvailabilityField;
            _pAvailabilityField = 0;
            HTMATCH_aligned_free(_pData);
            _pData = 0;
        }
        FORCE_INLINE uint32 getAllocatedCount() const FORCE_INLINE_END { return _uAllocatedCount; }
        FORCE_INLINE bool isEmpty() const FORCE_INLINE_END { return _uAllocatedCount == 0u; }
        FORCE_INLINE static constexpr size_t getOffsetFor(u32fast uIndex, u32fast uSlotByteSize) FORCE_INLINE_END {
            return size_t(uIndex) * size_t(uSlotByteSize);
        }
        FORCE_INLINE static constexpr u32fast getMaxQwordsFor(u32fast uSlotByteSize, u32fast uMaxCount) FORCE_INLINE_END {
            u32fast uTotalByteCount = uSlotByteSize * uMaxCount;
            return reqCountCoarseTo(uTotalByteCount, 6u);
        }
        FORCE_INLINE uint8* getDataFor(u32fast uIndex, u32fast uSlotByteSize) FORCE_INLINE_END {
            return _pData + getOffsetFor(uIndex, uSlotByteSize);
        }
        FORCE_INLINE const uint8* getDataFor(u32fast uIndex, u32fast uSlotByteSize) const FORCE_INLINE_END {
            return _pData + getOffsetFor(uIndex, uSlotByteSize);
        }
        uint8* allocateNewSlot(u32fast uSlotByteSize, u32fast uMaxSlotCount, u32fast* outIndex) {
            if (_uAllocatedCount >= uMaxSlotCount) {
                return 0;
            }
            u32fast uIndex = u32fast(_uNextIndex);
            *outIndex = uIndex;
            uint8* pResult = _pData + getOffsetFor(uIndex, uSlotByteSize);
            u32fast uQword = uIndex >> 6u;
            u32fast uBit = uIndex & 0x0000003Fu;
            uint64 uMask = 1uLL << uBit;
            uint64 uField = _pAvailabilityField[uQword];
            if ((uField & uMask) == 0uLL) {
                throw std::runtime_error("MemPageBase::allocateNewSlot : invalid state");
            }
            uField &= ~uMask;
            if (uField == 0uLL) {
                u32fast uMaxQwords = getMaxQwordsFor(uSlotByteSize, uMaxSlotCount);
                do {
                    uQword++;
                    uField = _pAvailabilityField[uQword];
                } while (uField == 0uLL && uQword < uMaxQwords);
                if (uQword >= uMaxQwords) {
                    throw std::runtime_error("MemPageBase::allocateNewSlot : invalid state");
                }
                uIndex = uQword << 6u;
            } else {
                uIndex++;
            }
            uBit = getTrailingZeroesCount64(uField) + 1u;
            if (uBit >= 64u || uBit < (uIndex & 0x0000003Fu)) {
                throw std::runtime_error("MemPageBase::allocateNewSlot : invalid state");
            }
            _uNextIndex = uint32((uQword << 6u) + uBit);
            _uAllocatedCount++;
            return pResult;
        };
        bool removeSlot(u32fast uIndex) {
            if (_uAllocatedCount) {
                u32fast uQword = uIndex >> 6u;
                u32fast uBit = uIndex & 0x0000003Fu;
                uint64 uMask = 1uLL << uBit;
                if (_pAvailabilityField[uQword] & uMask) {
                    throw new std::runtime_error("MemPageBase::removeSlot : invalid state");
                }
                _pAvailabilityField[uQword] |= uMask;
                if (uIndex < u32fast(_uNextIndex)) {
                    _uNextIndex = uint32(uIndex);
                }
                _uAllocatedCount--;
                return true;
            } else {
                return false;
            }
        }
    private:
        uint32 _uNextIndex;
        uint32 _uAllocatedCount;
        uint64* _pAvailabilityField;
        uint8* _pData;
    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // DefaultMemPage: mem page actually storing its size and count properties.
    //   In effect a simple wrapper around MemPageBase
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    class DefaultMemPage {
        DefaultMemPage(u32fast uSlotCount, u32fast uSlotByteSize, u8fast uAlignBits = 0u):
            _impl(uSlotCount, uSlotByteSize, uAlignBits),
            _uMaxSlotCount(uSlotCount), _uSlotByteSize(uSlotByteSize) {}
        ~DefaultMemPage() {}
        FORCE_INLINE uint32 getAllocatedCount() const FORCE_INLINE_END { return _impl.getAllocatedCount(); }
        FORCE_INLINE bool isEmpty() const FORCE_INLINE_END { return _impl.isEmpty(); }
        FORCE_INLINE uint32 getMaxCount() const FORCE_INLINE_END { return _uMaxSlotCount; }
        FORCE_INLINE uint32 getByteSize() const FORCE_INLINE_END { return _uSlotByteSize; }
        FORCE_INLINE bool isFull() const FORCE_INLINE_END { return _impl.getAllocatedCount() >= _uMaxSlotCount; }
        FORCE_INLINE uint8* getDataFor(u32fast uIndex) FORCE_INLINE_END {
            return _impl.getDataFor(uIndex, u32fast(_uSlotByteSize));
        }
        FORCE_INLINE const uint8* getDataFor(u32fast uIndex) const FORCE_INLINE_END {
            return _impl.getDataFor(uIndex, u32fast(_uSlotByteSize));
        }
        FORCE_INLINE uint8* allocateNewSlot(u32fast* outIndex) FORCE_INLINE_END {
            return _impl.allocateNewSlot(u32fast(_uSlotByteSize), u32fast(_uMaxSlotCount), outIndex);
        }
        FORCE_INLINE bool removeSlot(u32fast uIndex) FORCE_INLINE_END {
            return _impl.removeSlot(uIndex);
        }
    private:
        MemPageBase _impl;
        uint32 _uMaxSlotCount;
        uint32 _uSlotByteSize;
    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // StaticMemPage: mem page having its size and count properties known as template parameters.
    //   In effect a simple wrapper around MemPageBase
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    template<unsigned int T_COUNT, unsigned int T_SIZE>
    class StaticMemPage {
    public:
        StaticMemPage():_impl(T_COUNT, T_SIZE) {}
        StaticMemPage(u8fast uAlignBits):_impl(T_COUNT, T_SIZE, uAlignBits) {}
        ~StaticMemPage() {}
        FORCE_INLINE uint32 getAllocatedCount() const FORCE_INLINE_END { return _impl.getAllocatedCount(); }
        FORCE_INLINE bool isEmpty() const FORCE_INLINE_END { return _impl.isEmpty(); }
        FORCE_INLINE static constexpr u32fast getMaxCount() FORCE_INLINE_END { return T_COUNT; }
        FORCE_INLINE static constexpr u32fast getByteSize() FORCE_INLINE_END { return T_SIZE; }
        FORCE_INLINE bool isFull() const FORCE_INLINE_END { return _impl.getAllocatedCount() >= T_COUNT; }
        FORCE_INLINE uint8* getDataFor(u32fast uIndex) FORCE_INLINE_END {
            return _impl.getDataFor(uIndex, u32fast(T_SIZE));
        }
        FORCE_INLINE const uint8* getDataFor(u32fast uIndex) const FORCE_INLINE_END {
            return _impl.getDataFor(uIndex, u32fast(T_SIZE));
        }
        FORCE_INLINE uint8* allocateNewSlot(u32fast* outIndex) FORCE_INLINE_END {
            return _impl.allocateNewSlot(u32fast(T_SIZE), u32fast(T_COUNT), outIndex);
        }
        FORCE_INLINE bool removeSlot(u32fast uIndex) FORCE_INLINE_END {
            return _impl.removeSlot(uIndex);
        }
    private:
        MemPageBase _impl;
    };



} // namespace HTMATCH

#endif
