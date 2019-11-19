/* -----------------------------------
 * HTMATCH
 * PerfTest.cpp
 * -----------------------------------
 * Defines a perf test application entry point.
 * Not required to "use" VanillaHTM, but may give some handy usage examples
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

#define COMPILE_PERFTEST        1

#ifdef COMPILE_PERFTEST

#include "vanillaHTM/VanillaHTMConfig.h"

/*
#define VANILLA_SP_SUBNAMESPACE     LocalOptim16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_ALL_OPTIM
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE
*/

#define VANILLA_SP_SUBNAMESPACE     GlobalNoBoosting32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalNoBoosting16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalNoBoosting8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketNoBoosting32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketNoBoosting16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketNoBoosting8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalBoosted32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalBoosted16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalBoosted8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketBoosted32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketBoosted16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     BucketBoosted8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_BUCKET
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalNoBoosting32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalNoBoosting16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalNoBoosting8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOBOOSTING
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalDefault32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalDefault16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalDefault8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE
/*
#define VANILLA_SP_SUBNAMESPACE     LocalCorrectedRadius32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_CORRECTEDUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalCorrectedRadius16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_CORRECTEDUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalCorrectedRadius8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_CORRECTEDUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE
*/
#define VANILLA_SP_SUBNAMESPACE     LocalNoRadiusUpdate32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalNoRadiusUpdate16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     LocalNoRadiusUpdate8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

using namespace HTMATCH;

#include "examples/SampleTools.h"
#include <iostream>
#include <time.h>

template<class VanillaSPKind>
static void _reportPerfTest(const FixedDigitEncoder& inputEncoder, size_t uThousandsOfEpochs = 1u)
{
    static const char* tConfigTitles[11] = {
        "<unknown>",
        "Global inhib, noboost",                                                    // 1
        "Bucket inhib, noboost",                                                    // 2
        "Local inhib, noboost",                                                     // 3
        "Global inhib, boost 0.1",                                                  // 4
        "Bucket inhib, boost 0.1",                                                  // 5
        "Local inhib, boost 0.1",                                                   // 6
        "Local inhib, boost 0.1, correct dynamic radius",                           // 7
        "Local inhib, boost 0.1, no radius update",                                 // 8
        "Local inhib, boost 0.1, no radius update, optimized neighbor sampling",    // 9
        "Local inhib, boost 0.1, no radius update, gaussian filter test",           // 10
    };
    static const char* tSynapseKindTitles[4u] = {
        "<unknown>",
        "32b float",    // 1
        "16b FixPt",    // 2
        "8b FixPt",     // 3
    };

    static const size_t uQWordPerBinarySheet = VANILLA_HTM_SHEET_2DSIZE >> 6u;
    uint64* pInputBuffer = new uint64[uQWordPerBinarySheet * 4u];
    std::vector<uint16> vecActiveSPcolumns;
    vecActiveSPcolumns.reserve(64u);
    Rand inputDrawRNG;

    std::cout << "\nVanillaSP perf test - config : " << tConfigTitles[VanillaSPKind::getConfigIndex()]
        << " ; synapses permanence on : " << tSynapseKindTitles[VanillaSPKind::getSynapseKindIndex()] << std::endl;
    VanillaSPKind perfSp = VanillaSPKind(4u);
    std::cout << "\tInhibition radius at start:" << uint32(perfSp.getInhibitionRadius()) << std::endl;
    std::cout << "\tInit Done, now launching " << uThousandsOfEpochs << " thousand tight iterations..." << std::endl;
    uint64 uActiveSum = 0u;

    clock_t tClocksBefore = ::clock();
    size_t uRoundsToSpin = uThousandsOfEpochs * 1000u;
    for (size_t uEpoch = 0u; uEpoch < uRoundsToSpin; uEpoch++) {
        u8fast uRandCode6b = uint8(inputDrawRNG.getNext() & 0x003Fu);
        SDRTools::toBinaryBitmap64(inputEncoder.getInputVectorEncodingDigitCode(uRandCode6b),
            pInputBuffer, VANILLA_HTM_SHEET_BYTES_BINARY * 4u);
        perfSp.compute(pInputBuffer, vecActiveSPcolumns, true);
        uActiveSum += vecActiveSPcolumns.size();
    }
    clock_t tClocksAfter = ::clock();
    clock_t tClockDiff = tClocksAfter - tClocksBefore;
    std::cout << "\t\tPerformed " << uThousandsOfEpochs << " thousand epochs of this SP in "
        << ((uint64(tClockDiff)*1000uLL)/uint64(CLOCKS_PER_SEC)) << " milliseconds ("
        << ((double(tClockDiff)*1000.0)/(double(uRoundsToSpin)*double(CLOCKS_PER_SEC))) << " ms per iteration)" << std::endl;
    std::cout << "\t\tAverage active columns : " << double(uActiveSum)/double(uRoundsToSpin) << std::endl;
    std::cout << "\tInhibition radius at end:" << uint32(perfSp.getInhibitionRadius()) << std::endl;
    delete[] pInputBuffer;
}
; // template termination

#include <string>

int main()
{
    FixedDigitEncoder inputEncoder;

    _reportPerfTest<GlobalNoBoosting32::VanillaSP>(inputEncoder, 30u);
    _reportPerfTest<GlobalNoBoosting16::VanillaSP>(inputEncoder, 30u);
    _reportPerfTest<GlobalNoBoosting8::VanillaSP>(inputEncoder, 30u);

    _reportPerfTest<BucketNoBoosting32::VanillaSP>(inputEncoder, 20u);
    _reportPerfTest<BucketNoBoosting16::VanillaSP>(inputEncoder, 20u);
    _reportPerfTest<BucketNoBoosting8::VanillaSP>(inputEncoder, 20u);

    _reportPerfTest<GlobalBoosted32::VanillaSP>(inputEncoder, 30u);
    _reportPerfTest<GlobalBoosted16::VanillaSP>(inputEncoder, 30u);
    _reportPerfTest<GlobalBoosted8::VanillaSP>(inputEncoder, 30u);

    _reportPerfTest<BucketBoosted32::VanillaSP>(inputEncoder, 20u);
    _reportPerfTest<BucketBoosted16::VanillaSP>(inputEncoder, 20u);
    _reportPerfTest<BucketBoosted8::VanillaSP>(inputEncoder, 20u);

    _reportPerfTest<LocalNoBoosting32::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalNoBoosting16::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalNoBoosting8::VanillaSP>(inputEncoder, 5u);

    _reportPerfTest<LocalDefault32::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalDefault16::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalDefault8::VanillaSP>(inputEncoder, 5u);
/*
    _reportPerfTest<LocalCorrectedRadius32::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalCorrectedRadius16::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalCorrectedRadius8::VanillaSP>(inputEncoder, 5u);
*/
    _reportPerfTest<LocalNoRadiusUpdate32::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalNoRadiusUpdate16::VanillaSP>(inputEncoder, 5u);
    _reportPerfTest<LocalNoRadiusUpdate8::VanillaSP>(inputEncoder, 5u);

    std::cout << "\nAll Done." << std::endl;
    std::string sHop;
    std::cin >> sHop;

    return 0;
}

#endif // COMPILE_PERFTEST


