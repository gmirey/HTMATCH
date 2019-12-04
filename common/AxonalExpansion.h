/* -----------------------------------
 * HTMATCH
 * AxonalExpansion.h
 * -----------------------------------
 * Defines tools for dealing with axonal-expansion phases in HTMATCH for
 *   topological or semi-topological models
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
#ifndef _HTMATCH_AXONAL_EXPANSION_H
#define _HTMATCH_AXONAL_EXPANSION_H

#include "tools/system.h"
#include "tools/bittools.h"
#include "tools/mempage.h"
#include "tools/parallel.h"
#include <vector>

#define AXONAL_ARBOUR_SIZE_BUCKET_COUNT                     33u         // [1..16] then [18..32] step 2 then [36..64] step 4 then 70
#define AXONAL_ARBOUR_MAX_PAGES_PER_SIZE_BUCKET             512u        // 9b page index in bucket
#define AXONAL_ARBOUR_MAX_ARBORS_PER_PAGE                   65536u      // 16b index in page
#define AXONAL_ARBOUR_BYTES_PER_FIRST_EXPANSION_COUNT       35u         // 16b + 8b + (16 x 16b), in bytes
#define AXONAL_ARBOUR_LINE_SIZE                             64u         // standard cache line of 64 bytes
#define AXONAL_ARBOUR_LINE_SHIFT                            6u          // 64 is 2^6
#define AXONAL_ARBOUR_MAX_FIRST_EXPANSION_COUNT             128u        // how many shunting areas can a single arbor ignite
#define AXONAL_ARBOUR_MAX_LINES                             70u         // (128 * AXONAL_ARBOUR_BYTES_PER_FIRST_EXPANSION_COUNT) / 64

namespace HTMATCH {

    class AxonalArborMemManager {
    public:
        AxonalArborMemManager():_uAllocatedCount(0u) {
            for (u8fast uBucket = 0u; uBucket < AXONAL_ARBOUR_SIZE_BUCKET_COUNT; uBucket++) {
                _vecPagesPerSizeBucket[uBucket].reserve(AXONAL_ARBOUR_MAX_PAGES_PER_SIZE_BUCKET);
            }
        }
        ~AxonalArborMemManager() {
            for (u8fast uBucket = 0u; uBucket < AXONAL_ARBOUR_SIZE_BUCKET_COUNT; uBucket++) {
                for (auto itPage = _vecPagesPerSizeBucket[uBucket].begin(), itEnd = _vecPagesPerSizeBucket[uBucket].end();
                        itPage != itEnd; itPage++) {
                    MemPageBase* pPage = *itPage;
                    delete pPage;
                }
                _vecPagesPerSizeBucket[uBucket].clear();
            }
        }

        size_t getAllocatedCount() const { return _uAllocatedCount; }

        uint8* getArborDataFor(u32fast uAxonArborHandle, u8fast* outFirstExpansionCount) {
            u32fast uFirstExpansionCount = uAxonArborHandle >> 25u;
            if (uFirstExpansionCount) {
                u32fast uArborIndex = uAxonArborHandle & 0x01FFFFFFu;
                u32fast uBucket, uByteSize;
                _getBucketAndByteSizeFor(uFirstExpansionCount, &uBucket, &uByteSize);
                *outFirstExpansionCount = u8fast(uFirstExpansionCount);
                return _getArborDataKnowingSize(uBucket, uByteSize, uArborIndex);
            } else {
                return 0;
            }
        }
        const uint8* getArborDataFor(u32fast uAxonArborHandle, u8fast* outFirstExpansionCount) const {
            u32fast uFirstExpansionCount = uAxonArborHandle >> 25u;
            if (uFirstExpansionCount) {
                u32fast uArborIndex = uAxonArborHandle & 0x01FFFFFFu;
                u32fast uBucket, uByteSize;
                _getBucketAndByteSizeFor(uFirstExpansionCount, &uBucket, &uByteSize);
                *outFirstExpansionCount = u8fast(uFirstExpansionCount);
                return _getArborDataKnowingSize(uBucket, uByteSize, uArborIndex);
            } else {
                return 0;
            }
        }

        uint8* allocateNewArborData(u8fast uFirstExpansionCount, u32fast* outFullIndex) {
            u32fast uBucket, uByteSize;
            _getBucketAndByteSizeFor(uFirstExpansionCount, &uBucket, &uByteSize);
            return _allocateNewArborDataKnowingSize(uBucket, uByteSize, uFirstExpansionCount, outFullIndex);
        }

        uint8* reallocArborData(u32fast* inoutFullIndex, u8fast uNewFirstExpansionCount) {
            u32fast uOldIndex = *inoutFullIndex;
            u32fast uOldArborIndex = uOldIndex & 0x01FFFFFFu;
            u32fast uOldFirstExpansionCount = uOldIndex >> 25u;
            u32fast uOldBucket, uOldByteSize;
            _getBucketAndByteSizeFor(uOldFirstExpansionCount, &uOldBucket, &uOldByteSize);
            u32fast uNewBucket, uNewByteSize;
            _getBucketAndByteSizeFor(uNewFirstExpansionCount, &uNewBucket, &uNewByteSize);
            if (uOldBucket == uNewBucket) { // no realloc required
                return _getArborDataKnowingSize(uOldBucket, uOldByteSize, uOldArborIndex);
            } else {
                _removeArborDataKnowingSize(uOldBucket, uOldArborIndex);
                return _allocateNewArborDataKnowingSize(uNewBucket, uNewByteSize, uNewFirstExpansionCount, inoutFullIndex);
            }
        }

        void removeArborData(u32fast uAxonArborHandle) {
            u32fast uFirstExpansionCount = uAxonArborHandle >> 25u;
            if (uFirstExpansionCount) {
                u32fast uArborIndex = uAxonArborHandle & 0x01FFFFFFu;
                u32fast uBucket, unused;
                _getBucketAndByteSizeFor(uFirstExpansionCount, &uBucket, &unused);
                _removeArborDataKnowingSize(uBucket, uArborIndex);
            }
        }

    private:

        static constexpr u32fast getByteSizeFromFirstExpansionCount(u32fast uFirstExpansionCount) {
            return uFirstExpansionCount * AXONAL_ARBOUR_BYTES_PER_FIRST_EXPANSION_COUNT;
        }

        static constexpr u32fast getMinLineCountForByteSize(u32fast uByteSize) {
            return reqCountCoarseTo(uByteSize, 6u);
        }

        static constexpr u32fast getLineCountForBucket(u32fast uBucket) {
            const uint8 tPossibleResults[AXONAL_ARBOUR_SIZE_BUCKET_COUNT] =
                                                {  1u,  2u,  3u,  4u,  5u,  6u,  7u,  8u,
                                                   9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u,
                                                  18u, 20u, 22u, 24u, 26u, 28u, 30u, 32u,
                                                  36u, 40u, 44u, 48u, 52u, 56u, 60u, 64u,
                                                  70u
                                                };
            return u32fast(tPossibleResults[uBucket]);
        }

        static constexpr u32fast getBucketForMinLineCount(u32fast uMinLineCount) {
            const uint8 tPossibleResults[AXONAL_ARBOUR_MAX_LINES] =
                                                {  0u,  1u,  2u,  3u,  4u,  5u,  6u,  7u,
                                                   8u,  9u, 10u, 11u, 12u, 13u, 14u, 15u,
                                                  16u, 16u, 17u, 17u, 18u, 18u, 19u, 19u,
                                                  20u, 20u, 21u, 21u, 22u, 22u, 23u, 23u,
                                                  24u, 24u, 24u, 24u, 25u, 25u, 25u, 25u,
                                                  26u, 26u, 26u, 26u, 27u, 27u, 27u, 27u,
                                                  28u, 28u, 28u, 28u, 29u, 29u, 29u, 29u,
                                                  30u, 30u, 30u, 30u, 31u, 31u, 31u, 31u,
                                                  32u, 32u, 32u, 32u, 32u, 32u
                                                };
            return u32fast(tPossibleResults[uMinLineCount-1u]); // -1 as result for size 0 is not present in the table
        }

        static void _getBucketAndByteSizeFor(u32fast uFirstExpansionCount, u32fast* outBucket, u32fast* outByteSize) {
            u32fast uMinReqBytes = uFirstExpansionCount * AXONAL_ARBOUR_BYTES_PER_FIRST_EXPANSION_COUNT;
            u32fast uMinReqLines = reqCountCoarseTo(uFirstExpansionCount, 6u);
            if (uMinReqLines > AXONAL_ARBOUR_MAX_LINES) {
                throw std::invalid_argument(
                    "AxonalArborMemManager : expansion count shall not be greater to 128 and lines count is thus max 70");
            }
            u32fast uBucket = getBucketForMinLineCount(uMinReqLines);
            u32fast uLineCount = getLineCountForBucket(uBucket);
            *outBucket = uBucket;
            *outByteSize = uLineCount * AXONAL_ARBOUR_LINE_SIZE;
        }

        uint8* _getArborDataKnowingSize(u32fast uBucket, u32fast uByteSize, u32fast uArborIndex) {
            const std::vector<MemPageBase*>& vecPages = _vecPagesPerSizeBucket[uBucket];
            u32fast uPageIndex = uArborIndex >> 16u;
            u32fast uIndexInPage = uArborIndex & 0x0000FFFFu;
            return vecPages[uPageIndex]->getDataFor(uIndexInPage, uByteSize);
        }
        const uint8* _getArborDataKnowingSize(u32fast uBucket, u32fast uByteSize, u32fast uArborIndex) const {
            const std::vector<MemPageBase*>& vecPages = _vecPagesPerSizeBucket[uBucket];
            u32fast uPageIndex = uArborIndex >> 16u;
            u32fast uIndexInPage = uArborIndex & 0x0000FFFFu;
            return vecPages[uPageIndex]->getDataFor(uIndexInPage, uByteSize);
        }

        uint8* _allocateNewArborDataKnowingSize(u32fast uBucket, u32fast uByteSize, u32fast uExpansionCount,
            u32fast* outFullIndex)
        {
            std::vector<MemPageBase*>& vecPages = _vecPagesPerSizeBucket[uBucket];
            u32fast uPageIndex = 0u;
            for (auto itPage = vecPages.begin(), itEnd = vecPages.end(); itPage != itEnd; itPage++, uPageIndex++) {
                MemPageBase* pPage = *itPage;
                if (pPage->getAllocatedCount() < AXONAL_ARBOUR_MAX_ARBORS_PER_PAGE) {
                    return _allocateNewArborDataKnowingPage(pPage, uPageIndex, uBucket, uByteSize, uExpansionCount, outFullIndex);
                }
            }
            if (uPageIndex < AXONAL_ARBOUR_MAX_PAGES_PER_SIZE_BUCKET) {
                MemPageBase* pNewPage = new MemPageBase(AXONAL_ARBOUR_MAX_ARBORS_PER_PAGE, uByteSize);
                vecPages.push_back(pNewPage);
                return _allocateNewArborDataKnowingPage(pNewPage, uPageIndex, uBucket, uByteSize, uExpansionCount, outFullIndex);
            } else {
                return 0;
            }
        }
        uint8* _allocateNewArborDataKnowingPage(MemPageBase* pPage, u32fast uPageIndex, u32fast uBucket, u32fast uByteSize,
                u32fast uExpansionCount, u32fast* outFullIndex) {
            u32fast uIndexInPage;
            uint8* pResult = pPage->allocateNewSlot(uByteSize, AXONAL_ARBOUR_MAX_ARBORS_PER_PAGE, &uIndexInPage);
            if (pResult) {
                u32fast uFullIndex = uIndexInPage | (uPageIndex << 16u) | (uExpansionCount << 25u);
                *outFullIndex = uFullIndex;
                _uAllocatedCount++;
            }
            return pResult;
        }

        void _removeArborDataKnowingSize(u32fast uBucket, u32fast uArborIndex) {
            const std::vector<MemPageBase*>& vecPages = _vecPagesPerSizeBucket[uBucket];
            u32fast uPageIndex = uArborIndex >> 16u;
            u32fast uIndexInPage = uArborIndex & 0x0000FFFFu;
            if (vecPages[uPageIndex]->removeSlot(uIndexInPage)) {
                _uAllocatedCount--;
            }
        }

        size_t _uAllocatedCount;
        std::vector<MemPageBase*> _vecPagesPerSizeBucket[AXONAL_ARBOUR_SIZE_BUCKET_COUNT];
    };

    class SeqForwardExpand;
    class ParForwardExpand;

    template<class _Context>
    typedef void (*ForwardExpandFunc)(_Context* pContext,
        u16fast uBlockX, u16fast uBlockY, u16fast uBlockZ,
        u8fast uIndexInBlock, uint64 uParam, i32fast iParallelIndex);

    class ForwardExpandImpl {
    private:
        template<class _Context, typename Func>
        FORCE_INLINE static void _forwardExpandSecondPassImpl(_Context* pContext, const uint8* pAxonalArborData,
            u8fast uFirstByte, u8fast uSecondByte, u8fast uThirdByte,
            u16fast uOffsetX, u16fast uOffsetY, Func func, uint64 uParam, i32fast iParallelIndex) FORCE_INLINE_END
        {
            u8fast uShuntingDX = uFirstByte & 0x0Fu;                        // 4 lsb of 1st byte holds dX of shunting area
            u8fast uShuntingDY = uFirstByte >> 4u;                          // 4 msb of 1st byte holds dY of shunting area
            u8fast uShuntingZ = uSecondByte & 0x0Fu;                        // 4 lsb of 2nd byte holds Z of shunting area
            u8fast uSecondExpansionCount = uSecondByte >> 4u;               // 4 msb of 2nd byte holds second exp. count
            u16fast uSecondStepByteOffset = u16fast(uThirdByte) << 5u;      // 3rd byte holds half-page (=> x32Bytes) index
            u16fast uBlockStartX = uOffsetX + (u16fast(uShuntingDX) << 3u);
            u16fast uBlockStartY = uOffsetY + (u16fast(uShuntingDY) << 3u);
            u8fast uBlockStartZ = uShuntingZ << 2u;
            const uint8* pSecondStep = pAxonalArborData + uSecondStepByteOffset;
            for (u8fast uSecondStep = 0u; uSecondStep < uSecondExpansionCount; uSecondStep++) {
                u8fast uFirstByte = u8fast(*(pSecondStep++=);
                u8fast uIndexInBlock = u8fast(*(pSecondStep++=);
                u16fast uBlockDX = u16fast(uFirstByte & 0x0007u);          // 3 lsb holds dX
                u16fast uBlockDY = u16fast((uFirstByte >> 3u) & 0x0007u);  // 3 next holds dY
                u16fast uBlockDZ = u16fast(uFirstByte >> 6u);              // 2 msb holds dZ
                u16fast uBlockX = uBlockStartX + uBlockDX;
                u16fast uBlockY = uBlockStartY + uBlockDY;
                u16fast uBlockZ = uBlockStartZ + uBlockDZ;
                // ... and at last calls the operation to perform
                func(pContext, uBlockX, uBlockY, uBlockZ, uIndexInBlock, uParam, iParallelIndex);
            }
        }
        ; // template termination
        friend class SeqForwardExpand;
        friend class ParForwardExpand;
    };

    // Helper class for sequentially solving an axonal-arbor-expansion
    class SeqForwardExpand {
    public:

        // Asks for the expansion of a signal all the way to end of second-expansion pass, given an
        //   AxonalArborMemManager repository of those expansion definitions, and a function to be invoked
        //   to each final 'coordinate'
        template<class _Context, typename _Func>
        static void forwardExpandSignal(_Context* pContext, u32fast uAxonArbourHandle, AxonalArborMemManager* pArborDataMgr,
            u32fast uOffsetX, u32fast uOffsetY, _Func func, uint64 uParam)
        {
            u8fast uFirstExpansionCount;
            const uint8* pAxonalArborData = pArborDataMgr->getArborDataFor(uAxonArbourHandle, uFirstExpansionCount);
            if (uFirstExpansionCount && pAxonalArborData) {
                return SeqForwardExpand::_forwardExpandSignal(
                    pContext, uFirstExpansionCount, pAxonalArborData, uOffsetX, uOffsetY, func, uParam);
            } else {
                return 0;
            }
        }
        ; // template termination

    private:
        template<class _Context, typename _Func>
        static i32fast _forwardExpandSignal(_Context* pContext, u8fast uFirstExpansionCount, const uint8* pAxonalArborData,
            u32fast uOffsetX, u32fast uOffsetY, _Func func, uint64 uParam)
        {
            const uint8* pFirstStepHeader = pAxonalArborData;
            for (u8fast uFirstStep = 0u; uFirstStep < uFirstExpansionCount; uFirstStep++) {
                _forwardExpandSecondPassImpl(pContext, pAxonalArborData,
                    u8fast(*(pFirstStepHeader++)), u8fast(*(pFirstStepHeader++)), u8fast(*(pFirstStepHeader++)),
                    uOffsetX, uOffsetY, func, uParam, -1);
            }
            return -1;
        }
        ; // template termination

        friend class ParForwardExpand;
    };

    // Helper class for solving an axonal-arbor-expansion in parallel
    class ParForwardExpand {
    public:
        template<class _Context, typename _Func>
        static i32fast forwardExpandSignal(_Context* pContext, u32fast uAxonArbourHandle, AxonalArborMemManager* pArborDataMgr,
                u32fast uOffsetX, u32fast uOffsetY, _Func func, uint64 uParam)
        {
            u8fast uFirstExpansionCount;
            const uint8* pAxonalArborData = pArborDataMgr->getArborDataFor(uAxonArbourHandle, uFirstExpansionCount);
            if (uFirstExpansionCount && pAxonalArborData) {
                if (uFirstExpansionCount < 4u) {
                    return SeqForwardExpand::_forwardExpandSignal(
                        pContext, uFirstExpansionCount, pAxonalArborData, uOffsetX, uOffsetY, func, uParam);
                } else {
                    return ParForwardExpand::_forwardExpandSignal(
                        pContext, uFirstExpansionCount, pAxonalArborData, uOffsetX, uOffsetY, func, uParam);
                }
            } else {
                return 0;
            }
        }
        ; // template termination

    private:
        template<class _Context, typename _Func>
        static i32fast _forwardExpandSignal(_Context* pContext, u8fast uFirstExpansionCount, const uint8* pAxonalArborData,
            u32fast uOffsetX, u32fast uOffsetY, _Func func, uint64 uParam)
        {
            HTMATCH::for_count(HTMATCH_PAR, 0u, u32fast(uFirstExpansionCount),
                [pContext, pAxonalArborData, uOffsetX, uOffsetY, func, uParam](u32fast uFirstStep) {
                const uint8* pFirstStepHeader = pAxonalArborData + (uFirstStep*3u);
                _forwardExpandSecondPassImpl(pContext, pAxonalArborData,
                    u8fast(pFirstStepHeader[0]), u8fast(pFirstStepHeader[1]), u8fast(pFirstStepHeader[2]),
                    uOffsetX, uOffsetY, func, uParam, i32fast(uFirstStep));
            });
            return i32fast(uFirstExpansionCount);
        }
        ; // template termination

    };

    template<bool bParallel> struct ForwardExpandHelper {};
    template<> struct ForwardExpandHelper<false> { typedef SeqForwardExpand         Class; };
    template<> struct ForwardExpandHelper<true>  { typedef ParForwardExpand         Class; };


} // namespace HTMATCH

#endif // _HTMATCH_AXONAL_EXPANSION_H

