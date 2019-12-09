/* -----------------------------------
 * HTMATCH
 * TestExpansion.cpp
 * -----------------------------------
 * Defines a quick test application entry point for getting a first feeling
 *   of axonal expansion performance
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

//#define COMPILE_TESTEXPANSION        1

#ifdef COMPILE_TESTEXPANSION

#include "tools/system.h"
#include "tools/rand.h"
#include "common/AxonalExpansion.h"
#include <string>
#include <iostream>
#include <thread>
#include <time.h>

using namespace HTMATCH;

struct SegIntegrationZone {
    uint32 _uCount;
    uint32 _tList[8191u];
};

struct TestTopo {
    TestTopo(uint32 uCountX, uint32 uCountY, uint32 uCountZ):
        _uCountX(uCountX), _uCountY(uCountY), _uCountZ(uCountZ)
    {
        uint32 uTotalCount = _uCountX * _uCountY * _uCountZ;
        _pSegStates = new uint8[size_t(uTotalCount) * size_t(512u)];                    // max 512 segs per block
        _pAxonFwd = new uint16[size_t(uTotalCount) * (size_t(256u) * size_t(64u))];     // max 256 axons per block, x64 synapses
        _uCountZoneX = reqCountCoarseTo(uCountX, 3u);
        _uCountZoneY = reqCountCoarseTo(uCountY, 3u);
        _uCountZoneXY = _uCountZoneX * _uCountZoneY;
        _pSegIntegrationZones = new SegIntegrationZone[size_t(_uCountZoneXY) * size_t(128u)]; // 128 exp x zones
        _pAxonHandlesAndOffsets = 0;
    }
    ~TestTopo() {
        delete[] _pSegStates;
        delete[] _pAxonFwd;
        delete[] _pSegIntegrationZones;
        if (_pAxonHandlesAndOffsets)
            delete[] _pAxonHandlesAndOffsets;
    }
    void resetStates() {
        uint32 uTotalCount = _uCountX * _uCountY * _uCountZ;
        std::memset(_pSegStates, 0u, size_t(uTotalCount) * size_t(512u));
    }
    void resetIntgZones() {
        uint32 uTotalZoneCount = _uCountZoneX * _uCountZoneY;
        std::memset(_pSegIntegrationZones, 0u, size_t(uTotalZoneCount) * size_t(128u));
    }
    void resetAll() {
        resetStates();
        resetIntgZones();
    }
    void allocateAxonalTables(uint32 uAxonCount) {
        if (_pAxonHandlesAndOffsets)
            delete[] _pAxonHandlesAndOffsets;
        _pAxonHandlesAndOffsets = new uint64[uAxonCount];
    }
    FORCE_INLINE void initOffsetAndHandleFor(uint32 uAxon, uint64 uOffsetAndHandle) FORCE_INLINE_END {
        _pAxonHandlesAndOffsets[uAxon] = uOffsetAndHandle; }
    FORCE_INLINE uint64 getOffsetAndHandleFor(uint32 uAxon) const FORCE_INLINE_END { return _pAxonHandlesAndOffsets[uAxon]; }

    FORCE_INLINE static void doHtmExpansionToSeg(TestTopo* pContext,
            uint32 uPackedBlockHandle,
            u8fast uAxonIndexInBlock, uint64 uParam, i32fast) FORCE_INLINE_END {
        pContext->_doHtmExpansionToSeg(uPackedBlockHandle, uAxonIndexInBlock, uParam);
    }
    FORCE_INLINE static void doHtmExpansionToList(TestTopo* pContext,
            uint32 uPackedBlockHandle,
            u8fast uAxonIndexInBlock, uint64 uParam, i32fast iParallelIndex) FORCE_INLINE_END {
        pContext->_doHtmExpansionToList(uPackedBlockHandle, uAxonIndexInBlock, uParam, iParallelIndex);
    }
    FORCE_INLINE uint32 getZoneCountX() const FORCE_INLINE_END { return _uCountZoneX; } 
    FORCE_INLINE uint32 getZoneCountY() const FORCE_INLINE_END { return _uCountZoneY; }
    uint64 getIterResultsAndClear() {
        uint32 uTotalCount = _uCountX * _uCountY * _uCountZ;
        size_t uTotalSegCount = size_t(uTotalCount) * size_t(512u);
        uint64 uResult = 0uLL;
        for (size_t uSeg = 0u; uSeg < uTotalSegCount; uSeg++) {
            uResult += uint32(_pSegStates[uSeg]);
        }
        std::memset(_pSegStates, 0u, uTotalSegCount);
        return uResult;
    }

    FORCE_INLINE void intgListsToSegs(uint32 uZoneX, uint32 uZoneY, i32fast iMaxExp) FORCE_INLINE_END {
        uint32 uZoneOffsetX = uZoneX << 3u;
        uint32 uZoneOffsetY = uZoneY << 3u;
        for (i32fast iExp = 0; iExp < iMaxExp; iExp++) {
            size_t uZoneId = iExp * _uCountZoneXY + uZoneX * _uCountZoneY + uZoneY;
            uint32 uCount = _pSegIntegrationZones[uZoneId]._uCount;
            const uint32* pSegList = _pSegIntegrationZones[uZoneId]._tList;
            for (uint32 uSeg = 0u; uSeg < uCount; uSeg++) {
                uint32 uPackedVal = pSegList[uSeg];
                uint32 uIntgParam = uPackedVal >> 22u;
                uint32 uSegIndexInBlock = uPackedVal & 0x000001FFu;
                uint32 uBlockX = ((uPackedVal >> 9u) & 0x00000007u) + uZoneOffsetX;
                uint32 uBlockY = ((uPackedVal >> 12u) & 0x00000007u) + uZoneOffsetY;
                uint32 uBlockZ = ((uPackedVal >> 15u) & 0x0000007Fu);
                size_t uSegIndex = _getSegIndex(uBlockX, uBlockY, uBlockZ, uSegIndexInBlock);
                uint8 uCurrentState = _pSegStates[uSegIndex];
                uint8 uUpdtState = uCurrentState + uint8(uIntgParam);
                _pSegStates[uSegIndex] = uUpdtState;
            }
        }
    }


private:
    FORCE_INLINE size_t _getSegIndex(uint32 uBlockX, uint32 uBlockY, uint32 uBlockZ, uint16 uSegIdInBlock) const FORCE_INLINE_END {
        return (size_t((uBlockX * _uCountY + uBlockY) * _uCountZ + uBlockZ) << 9u) | size_t(uSegIdInBlock);
    }
    FORCE_INLINE size_t _getAxonOffset(uint32 uBlockX, uint32 uBlockY, uint32 uBlockZ, u8fast uAxonIdInBlock) const FORCE_INLINE_END {
        return (size_t((uBlockX * _uCountY + uBlockY) * _uCountZ + uBlockZ) << 8u) | size_t(uAxonIdInBlock);
    }

    FORCE_INLINE void _doHtmExpansionToSeg(uint32 uPackedBlockHandle,
            u8fast uAxonIndexInBlock, uint64 uParam) FORCE_INLINE_END {
        uint32 uBlockX = uPackedBlockHandle & 0x00001FFFu;
        uint32 uBlockY = (uPackedBlockHandle >> 13u) & 0x00001FFFu;
        uint32 uBlockZ = uPackedBlockHandle >> 26u;
        size_t uOffset = _getAxonOffset(uBlockX, uBlockY, uBlockZ, uAxonIndexInBlock);
        const uint16* pSynInAxon = _pAxonFwd + (uOffset * size_t(64u));
        for (uint16 uSyn = 0u; uSyn < 64u; uSyn++) {
            uint16 uSynData = pSynInAxon[uSyn];
            uint32 uSegOffset = uint32(uSynData & 0x0007u);
            uint16 uSegIndexInBlock = (uSynData >> 3u) & 0x01FFu;
            uint16 uSynPerm = uSynData >> 12u;
            uint32 uSegBlockHandle = uPackedBlockHandle + expandXYZ<1u,1u,1u, 0u, 13u, 26u>(uSegOffset);
            _intgDirectToSeg(uSegBlockHandle, uSegIndexInBlock, uSynPerm, uParam);
        }
    }
    FORCE_INLINE void _doHtmExpansionToList(uint32 uPackedBlockHandle,
            u8fast uAxonIndexInBlock, uint64 uParam, i32fast iParallelIndex) FORCE_INLINE_END {
        uint32 uBlockX = uPackedBlockHandle & 0x00001FFFu;
        uint32 uBlockY = (uPackedBlockHandle >> 13u) & 0x00001FFFu;
        uint32 uBlockZ = uPackedBlockHandle >> 26u;
        size_t uOffset = _getAxonOffset(uBlockX, uBlockY, uBlockZ, uAxonIndexInBlock);
        const uint16* pSynInAxon = _pAxonFwd + (uOffset * size_t(64u));
        for (uint16 uSyn = 0u; uSyn < 64u; uSyn++) {
            uint16 uSynData = pSynInAxon[uSyn];
            uint32 uSegOffset = uint32(uSynData & 0x0007u);
            uint16 uSegIndexInBlock = (uSynData >> 3u) & 0x01FFu;
            uint16 uSynPerm = uSynData >> 12u;
            uint32 uSegBlockHandle = uPackedBlockHandle + expandXYZ<1u,1u,1u, 0u, 13u, 26u>(uSegOffset);
            _toIntgList(uSegBlockHandle, uSegIndexInBlock, uSynPerm, uParam, iParallelIndex);
        }
    }

    FORCE_INLINE void _intgDirectToSeg(uint32 uPackedBlockHandle, uint16 uSegIndexInBlock,
            uint16 uSynPerm, uint64 uParam) FORCE_INLINE_END {
        uint32 uBlockX = uPackedBlockHandle & 0x00001FFFu;
        uint32 uBlockY = (uPackedBlockHandle >> 13u) & 0x00001FFFu;
        uint32 uBlockZ = uPackedBlockHandle >> 26u;
        size_t uSegIndex = _getSegIndex(uBlockX, uBlockY, uBlockZ, uSegIndexInBlock);
        uint8 uCurrentState = _pSegStates[uSegIndex];
        uint8 uUpdtState = _intgState(uCurrentState, uSynPerm, uParam);
        _pSegStates[uSegIndex] = uUpdtState;
    }

    FORCE_INLINE void _toIntgList(uint32 uPackedBlockHandle, uint16 uSegIndexInBlock,
            uint16 uSynPerm, uint64 uParam, i32fast iParallelIndex) FORCE_INLINE_END {
        uint32 uBlockX = uPackedBlockHandle & 0x00001FFFu;
        uint32 uBlockY = (uPackedBlockHandle >> 13u) & 0x00001FFFu;
        uint32 uBlockZ = uPackedBlockHandle >> 26u;
        uint32 uZoneX = uBlockX >> 3u;
        uint32 uZoneY = uBlockY >> 3u;
        uint32 uInZoneId = ((uBlockX & 0x00000007u) << 9u) | ((uBlockY & 0x00000007u) << 12u) |
            (uBlockZ << 15u) | uint32(uSegIndexInBlock);
        _addToZoneList(uZoneX, uZoneY, iParallelIndex, uInZoneId, uSynPerm, uint32(uParam));
    }

    FORCE_INLINE void _addToZoneList(uint32 uZoneX, uint32 uZoneY, i32fast iParallelIndex, uint32 uInZoneId,
            uint16 uSynPerm, uint64 uParam) FORCE_INLINE_END {
        size_t uZoneId = iParallelIndex * _uCountZoneXY + uZoneX * _uCountZoneY + uZoneY;
        uint32 uPackedParam = TestTopo::_getIntgValue(uSynPerm, uParam);
        uint32 uCount = _pSegIntegrationZones[uZoneId]._uCount;
        _pSegIntegrationZones[uZoneId]._tList[uCount] = uInZoneId | (uPackedParam << 22u);
        _pSegIntegrationZones[uZoneId]._uCount = uCount+1u;
    }

    FORCE_INLINE uint8 _intgState(uint8 uCurrentState, uint16 uSynPerm, uint64 uParam) FORCE_INLINE_END {
        return uCurrentState + uint8(TestTopo::_getIntgValue(uSynPerm, uParam));
    }

    FORCE_INLINE static constexpr uint32 _getIntgValue(uint16 uSynPerm, uint64 uParam) FORCE_INLINE_END {
        int32 iNegIfPermGreaterOrEqFour = 3 - int32(uSynPerm);
        uint32 uOneIfPermGreaterOrEqFour = uint32(iNegIfPermGreaterOrEqFour) >> 31u;
        return uOneIfPermGreaterOrEqFour;
    }

    uint32 _uCountX;
    uint32 _uCountY;
    uint32 _uCountZ;
    uint32 _uCountZoneX;
    uint32 _uCountZoneY;
    uint32 _uCountZoneXY;
    uint8* _pSegStates;
    uint16* _pAxonFwd;
    SegIntegrationZone* _pSegIntegrationZones;
    uint64* _pAxonHandlesAndOffsets;
};

static TestTopo* _testInitArbors(AxonalArborMemManager* pAxonalArborMemMgr, uint32 uAxonCountX, uint32 uAxonCountY, uint32 uAxonCountZ,
    uint32 uMaxSpread, bool bSparseSpread) {
    uint32 uCountX = uAxonCountX + uMaxSpread*2u;
    uint32 uCountY = uAxonCountY + uMaxSpread*2u;

    TestTopo* pTestTopo = new TestTopo(uCountX, uCountY, 64u);
    pTestTopo->allocateAxonalTables(uAxonCountX * uAxonCountY * uAxonCountZ);
    // TODO : init
    return pTestTopo;
}

int main()
{

    AxonalArborMemManager* pAxonalArborMemMgr = new AxonalArborMemManager();
    Rand rng;
    uint64 uResult, uReallyDone;
    clock_t tClocksBefore, tClocksAfter, tClockDiff;

    static const uint32 uAxonCountX = 250u;
    static const uint32 uAxonCountY = 250u;
    static const uint32 uAxonCountZ = 100u;
    
    static const uint32 uTotalAxonCount = uAxonCountX * uAxonCountY * uAxonCountZ;  // 6 250 000

    static const uint32 uIterationCount = 1'000'000u;
    static const uint32 uMeanActivCount = 16u;
    static const uint32 uActivVariation = 8u;


    std::cout << "Now initializing arbors for " << uTotalAxonCount << " axons (efferent from about 1 square centimeter cortex)" << std::endl;
    TestTopo* pResultingTopo = _testInitArbors(pAxonalArborMemMgr, uAxonCountX, uAxonCountY, uAxonCountZ, 40u, true);

    std::cout << "\nResetting states..." << std::endl;
    pResultingTopo->resetAll();
    rng.seed(42u);
    uResult = 0uLL;
    std::cout << "\nNow checking " << uIterationCount << " x " << uMeanActivCount << " avg expansion phases sequentially." << std::endl;
    uReallyDone = 0u;
    tClocksBefore = ::clock();
    for (uint64 uDraw = 0u; uDraw < uIterationCount; uDraw++) {
        uint32 uActivThisTime = rng.drawNextFromZeroToExcl(uActivVariation*2+1) + uMeanActivCount - uActivVariation;
        for (uint32 uActiv = 0u; uActiv < uActivThisTime; uActiv++) {
            uint32 uAxon = rng.drawNextFromZeroToExcl(uTotalAxonCount);
            uint64 uOffsetAndHandle = pResultingTopo->getOffsetAndHandleFor(uAxon);
            SeqForwardExpand::forwardExpandSignal(pResultingTopo, uint32(uOffsetAndHandle >> 32u), 
                pAxonalArborMemMgr, uint32(uOffsetAndHandle), TestTopo::doHtmExpansionToSeg, 0u);
        }
        uReallyDone += uint64(uActivThisTime);
        uResult += pResultingTopo->getIterResultsAndClear();
    }
    tClocksAfter = ::clock();
    tClockDiff = tClocksAfter - tClocksBefore;
    std::cout << "\tPerformed " << uReallyDone << " expansion phases in "
        << ((uint64(tClockDiff) * 1000uLL) / uint64(CLOCKS_PER_SEC)) << " milliseconds ("
        << ((double(tClockDiff)*1000000000.0)/(double(uReallyDone)*double(CLOCKS_PER_SEC))) << " nanoseconds per expansion)"
        << std::endl;
    std::cout << "\tResult = " << uResult << std::endl;
    clock_t tClockDiffSeq = tClockDiff;

    std::cout << "\nResetting states..." << std::endl;
    pResultingTopo->resetAll();
    rng.seed(42u);
    uResult = 0uLL;
    std::cout << "\nNow checking " << uIterationCount << " x " << uMeanActivCount << " avg expansion phases, each asked to be solved in parallel. " << std::endl;
    int iNumberOfThreads = std::thread::hardware_concurrency();
    std::cout << "\tNumber of hardware threads detected at runtime = " << iNumberOfThreads << std::endl;
    int iPlausibleNumberOfCores = iNumberOfThreads > 2 && 0 == (iNumberOfThreads & 1) ? (iNumberOfThreads >> 1) : iNumberOfThreads;
    std::cout << "\tPlausible number of cores = " << iPlausibleNumberOfCores << std::endl;
    uReallyDone = 0u;
    uint32 uZoneCountX = pResultingTopo->getZoneCountX();
    uint32 uZoneCountY = pResultingTopo->getZoneCountY();
    uint32 uBaseZoneXperRound = uZoneCountX / 3u;
    uint32 uBaseZoneYperRound = uZoneCountY / 3u;
    tClocksBefore = ::clock();
    for (uint64 uDraw = 0u; uDraw < uIterationCount; uDraw++) {
        uint32 uActivThisTime = rng.drawNextFromZeroToExcl(uActivVariation*2+1) + uMeanActivCount - uActivVariation;
        i32fast iMaxExp = 0u;
        for (uint32 uActiv = 0u; uActiv < uActivThisTime; uActiv++) {
            uint32 uAxon = rng.drawNextFromZeroToExcl(uTotalAxonCount);
            uint64 uOffsetAndHandle = pResultingTopo->getOffsetAndHandleFor(uAxon);
            i32fast iExpThisTime = ParForwardExpand::forwardExpandSignal(pResultingTopo, uint32(uOffsetAndHandle >> 32u), 
                pAxonalArborMemMgr, uint32(uOffsetAndHandle), TestTopo::doHtmExpansionToList, 0u);
            if (iMaxExp < iExpThisTime)
                iMaxExp = iExpThisTime;
        }
        for (uint32 uOffsetZoneX = 0u; uOffsetZoneX < 3u; uOffsetZoneX++) {
            for (uint32 uOffsetZoneY = 0u; uOffsetZoneY < 3u; uOffsetZoneY++) {
                uint32 uZoneXthisRound = uBaseZoneXperRound;
                uint32 uZoneYthisRound = uBaseZoneYperRound;
                if (uZoneXthisRound * 3u + uOffsetZoneX < uZoneCountX)
                    uZoneXthisRound++;
                if (uZoneYthisRound * 3u + uOffsetZoneY < uZoneCountY)
                    uZoneYthisRound++;
                uint32 uZonesThisRound = uZoneXthisRound * uZoneYthisRound;
                HTMATCH::for_count(HTMATCH_PAR, 0u, uZonesThisRound,
                        [uOffsetZoneX, uOffsetZoneY, uZoneXthisRound, pResultingTopo, iMaxExp](u32fast uZoneXY){
                    uint32 uZoneX = uZoneXY / uZoneXthisRound;
                    uint32 uZoneY = uZoneXY % uZoneXthisRound;
                    uZoneX *= 3u;
                    uZoneY *= 3u;
                    uZoneX += uOffsetZoneX;
                    uZoneY += uOffsetZoneY;
                    pResultingTopo->intgListsToSegs(uZoneX, uZoneY, iMaxExp);
                });
            }
        }
        uReallyDone += uint64(uActivThisTime);
        uResult += pResultingTopo->getIterResultsAndClear();
    }
    tClocksAfter = ::clock();
    tClockDiff = tClocksAfter - tClocksBefore;
    std::cout << "\tPerformed " << uReallyDone << " expansion phases in "
        << ((uint64(tClockDiff) * 1000uLL) / uint64(CLOCKS_PER_SEC)) << " milliseconds ("
        << ((double(tClockDiff)*1000000000.0)/(double(uReallyDone)*double(CLOCKS_PER_SEC))) << " nanoseconds per expansion)"
        << std::endl;
    if (tClockDiff)
        std::cout << "\tSpeedup ratio = " << double(tClockDiffSeq) / double(tClockDiff) << std::endl;
    std::cout << "\tResult = " << uResult << std::endl;

    std::cout << "\nAll Done." << std::endl;
    std::string sHop;
    std::cin >> sHop;

    delete pResultingTopo;
    delete pAxonalArborMemMgr;

    return 0;
}

#endif // COMPILE_TESTEXPANSION


