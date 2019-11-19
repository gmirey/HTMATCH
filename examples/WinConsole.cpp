/* -----------------------------------
 * HTMATCH
 * WinConsole.cpp
 * -----------------------------------
 * Defines a test application entry point for MS Windows.
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

//#define COMPILE_WINCONSOLE        1

#ifdef COMPILE_WINCONSOLE

#define HTMATCH_WIN_TEST_MODE_FULLAUTO         1        // runs as fast as possible with a random valid input
#define HTMATCH_WIN_TEST_MODE_STEP_RAND        2        // draws a new random input on user keypress and iterate once
#define HTMATCH_WIN_TEST_MODE_STEP_USER_INPUT  3        // uses user keypress as input and iterate once
#define HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT  4        // uses last user keypress as input but keeps iterating on it

// Feel free to define TEST_ITERATION_MODE at will to one of the options above
#define TEST_ITERATION_MODE                        HTMATCH_WIN_TEST_MODE_FULLAUTO

#if defined(_WIN32) || defined(_WIN64)        // Wouldn't work for other target OSes

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

// make sure to include WinGDITools (including windows headers) after the vanillaHTM generators...
//   (otherwise we'd have conflict between windows min and max macros and std defs)

#include "examples/SampleTools.h"    
#include "examples/WinGDITools.h"    

namespace HTMATCH {

    class TestApp {
    public:
        TestApp():
            _singleRand(42u),
            _gnb32(k_uInputSheets), _gnb16(k_uInputSheets), _gnb8(k_uInputSheets),
            _bnb32(k_uInputSheets), _bnb16(k_uInputSheets), _bnb8(k_uInputSheets),
            _lnb32(k_uInputSheets), _lnb16(k_uInputSheets), _lnb8(k_uInputSheets),
            _g32(k_uInputSheets),   _g16(k_uInputSheets),   _g8(k_uInputSheets),
            _b32(k_uInputSheets),   _b16(k_uInputSheets),   _b8(k_uInputSheets),
            _l32(k_uInputSheets),   _l16(k_uInputSheets),   _l8(k_uInputSheets),
            _lnu32(k_uInputSheets), _lnu16(k_uInputSheets), _lnu8(k_uInputSheets),
            _inputEncoder(), _pInputBuffer(0), _pTmpOutputBuffer(0), _pTmpMinBuffer(0), _pTableBitmaps(0)
        {
            _pInputBuffer = new uint64[k_uQWordPerBinarySheet * k_uInputSheets];
            _pTmpOutputBuffer = new uint64[k_uQWordPerBinarySheet];
            _pTmpMinBuffer = new uint32[VANILLA_HTM_SHEET_2DSIZE];
            _pTableBitmaps = new Gdiplus::Bitmap*[k_uTotalBitmapsCount];
            for (size_t uBitmap = 0u; uBitmap < k_uTotalBitmapsCount; uBitmap++) {
                _pTableBitmaps[uBitmap] = new Gdiplus::Bitmap(VANILLA_HTM_SHEET_WIDTH, VANILLA_HTM_SHEET_HEIGHT, PixelFormat24bppRGB);
            }
        }

        ~TestApp()
        {
            for (size_t uBitmap = 0u; uBitmap < k_uTotalBitmapsCount; uBitmap++) {
                delete _pTableBitmaps[uBitmap];
            }
            delete[] _pTableBitmaps;
            delete[] _pTmpMinBuffer;
            delete[] _pTmpOutputBuffer;
            delete[] _pInputBuffer;
        }

        #define COMPUTE_USING(sp, uSpIndex) do {\
            sp.compute(_pInputBuffer, vecActiveSPcolumns, true, _pTmpOutputBuffer, _pTmpMinBuffer); \
            size_t uBitmapOffset = k_uInputBitmapsCount + uSpIndex * k_uOutputBitmapsPerConf; \
            GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 0u], sp.getRawActivationLevels(), \
                ColorTools::color1K<uint16>); \
            if (sp.doesUseBoosting()) { \
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 1u], sp.getBoostingFactors(), \
                    ColorTools::colorBoosting); \
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 2u], sp.getBoostedActivationLevels(), \
                    ColorTools::color256K); \
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 3u], _pTmpMinBuffer, \
                    ColorTools::color256K); \
            } else { \
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 3u], _pTmpMinBuffer, \
                    ColorTools::color1K<uint16>); \
            } \
            GDI::GDITools::_toDisplayBitmapFromBinaryOnSheet(_pTableBitmaps[uBitmapOffset + 4u], _pTmpOutputBuffer, \
                255u, 255u, 255u); \
        } while (0)

        void runIteration()
        {
            const std::vector<uint16>* pVecActiveInputIndices;
            std::vector<uint16> vecActiveSPcolumns;
            vecActiveSPcolumns.reserve(64u);
            if (GDI::GDITools::_bCancelInput) {
#if (TEST_ITERATION_MODE != HTMATCH_WIN_TEST_MODE_FULLAUTO) && (TEST_ITERATION_MODE != HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT)
                return;
#else
                pVecActiveInputIndices = &(_inputEncoder.getInputVectorEncodingDigitCode(64u));
#endif
            } else {
#if (TEST_ITERATION_MODE != HTMATCH_WIN_TEST_MODE_FULLAUTO) && (TEST_ITERATION_MODE != HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT)
                if (GDI::GDITools::_uLastCharTicks > 0u)
                    return;
                else
                    GDI::GDITools::_uLastCharTicks++;
#endif

#if (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_FULLAUTO) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_RAND)
                uint8 uRandCode6b = uint8(_singleRand.getNext() & 0x003Fu);
                pVecActiveInputIndices = &(_inputEncoder.getInputVectorEncodingDigitCode(uRandCode6b));
#else
                uint8 uDigitCode6b = _inputEncoder.getDigitCode6bFromChar(unsigned char(GDI::GDITools::_cLastChar));
                pVecActiveInputIndices = &(_inputEncoder.getInputVectorEncodingDigitCode(uDigitCode6b));
#endif
            }

            SDRTools::toBinaryBitmap64(*pVecActiveInputIndices, _pInputBuffer, VANILLA_HTM_SHEET_BYTES_BINARY * k_uInputSheets);

            COMPUTE_USING(_gnb32,  0u);
            COMPUTE_USING(_gnb16,  1u);
            COMPUTE_USING(_gnb8,   2u);
            COMPUTE_USING(_bnb32,  3u);
            COMPUTE_USING(_bnb16,  4u);
            COMPUTE_USING(_bnb8,   5u);
            COMPUTE_USING(_lnb32,  6u);
            COMPUTE_USING(_lnb16,  7u);
            COMPUTE_USING(_lnb8,   8u);
            COMPUTE_USING(_g32,    9u);
            COMPUTE_USING(_g16,   10u);
            COMPUTE_USING(_g8,    11u);
            COMPUTE_USING(_b32,   12u);
            COMPUTE_USING(_b16,   13u);
            COMPUTE_USING(_b8,    14u);
            COMPUTE_USING(_l32,   15u);
            COMPUTE_USING(_l16,   16u);
            COMPUTE_USING(_l8,    17u);
            COMPUTE_USING(_lnu32, 18u);
            COMPUTE_USING(_lnu16, 19u);
            COMPUTE_USING(_lnu8,  20u);

            for (size_t uInput = 0u; uInput < k_uInputSheets; uInput++) {
                GDI::GDITools::_toDisplayBitmapFromBinaryOnSheet(_pTableBitmaps[uInput], _pInputBuffer+(uInput*k_uQWordPerBinarySheet), 50u, 50u, 255u);
            }

            size_t uBitmapToDraw = 0u;
            for (; uBitmapToDraw < k_uInputBitmapsCount; uBitmapToDraw++) {
                size_t uPosX = 0u;
                size_t uPosY = uBitmapToDraw;
                int iStartX = 3 + int(uPosX)*67;
                int iStartY = 2 + int(uPosY)*35;
                GDI::GDITools::_pGraphics->DrawImage(_pTableBitmaps[uBitmapToDraw], iStartX, iStartY);
            }
            for (size_t uConfCategory = 0u; uConfCategory < k_uOutputConfCategories; uConfCategory++) {
                size_t uPosX = 1u + uConfCategory;
                int iStartX = 3 + int(uPosX)*67;
                for (size_t uConfInCategory = 0u; uConfInCategory < k_uOutputBitConfPerConfCategory; uConfInCategory++) {
                    for (size_t uBitmapPerConf = 0u; uBitmapPerConf < k_uOutputBitmapsPerConf; uBitmapPerConf++, uBitmapToDraw++) {
                        int iStartY = 2 + int(uBitmapPerConf)*35 + 180*int(uConfInCategory);
                        GDI::GDITools::_pGraphics->DrawImage(_pTableBitmaps[uBitmapToDraw], iStartX, iStartY);
                    }
                }
            }

        }

    private:
        Rand _singleRand;

        GlobalNoBoosting32::VanillaSP _gnb32;
        GlobalNoBoosting16::VanillaSP _gnb16;
        GlobalNoBoosting8::VanillaSP _gnb8;

        BucketNoBoosting32::VanillaSP _bnb32;
        BucketNoBoosting16::VanillaSP _bnb16;
        BucketNoBoosting8::VanillaSP _bnb8;

        LocalNoBoosting32::VanillaSP _lnb32;
        LocalNoBoosting16::VanillaSP _lnb16;
        LocalNoBoosting8::VanillaSP _lnb8;

        GlobalBoosted32::VanillaSP _g32;
        GlobalBoosted16::VanillaSP _g16;
        GlobalBoosted8::VanillaSP _g8;

        BucketBoosted32::VanillaSP _b32;
        BucketBoosted16::VanillaSP _b16;
        BucketBoosted8::VanillaSP _b8;

        LocalDefault32::VanillaSP _l32;
        LocalDefault16::VanillaSP _l16;
        LocalDefault8::VanillaSP _l8;

        LocalNoRadiusUpdate32::VanillaSP _lnu32;
        LocalNoRadiusUpdate16::VanillaSP _lnu16;
        LocalNoRadiusUpdate8::VanillaSP _lnu8;

        FixedDigitEncoder _inputEncoder;
        uint64* _pInputBuffer;
        uint64* _pTmpOutputBuffer;
        uint32* _pTmpMinBuffer;
        Gdiplus::Bitmap** _pTableBitmaps;

        static const size_t k_uQWordPerBinarySheet = VANILLA_HTM_SHEET_2DSIZE >> 6u;
        static const size_t k_uInputSheets = 4u;
        static const size_t k_uInputBitmapsCount = k_uInputSheets;
        static const size_t k_uOutputBitmapsPerConf = 5u;
        static const size_t k_uOutputBitConfPerConfCategory = 3u;
        static const size_t k_uOutputConfCategories = 7u;
        static const size_t k_uOutputBitmapCount = k_uOutputBitmapsPerConf * k_uOutputBitConfPerConfCategory * k_uOutputConfCategories;
        static const size_t k_uTotalBitmapsCount = k_uInputBitmapsCount + k_uOutputBitmapCount;
    };


} // namespace HTMATCH

using namespace HTMATCH;

HWND                 GDI::GDITools::_hwnd;
uint32               GDI::GDITools::_uLastCharTicks;
bool                 GDI::GDITools::_bContinue;
bool                 GDI::GDITools::_bCancelInput;
wchar_t              GDI::GDITools::_cLastChar;
Gdiplus::BitmapData  GDI::GDITools::_BmpData;
ULONG_PTR            GDI::GDITools::_gdiplusToken;
Gdiplus::Graphics*   GDI::GDITools::_pGraphics;

int main()
{
    if (!GDI::GDITools::Init())
        return -1;

    TestApp* pTestApp = new TestApp();

    MSG Msg;
    do {

        pTestApp->runIteration();

        while (PeekMessage(&Msg, NULL, 0, 0, 0)) {
            if (GetMessage(&Msg, NULL, 0, 0)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }

    } while (GDI::GDITools::_bContinue);

    delete pTestApp;

    GDI::GDITools::Shutdown();
    return 0;
}


#endif // _WIN32 or _WIN64

#endif // COMPILE_WINCONSOLE

