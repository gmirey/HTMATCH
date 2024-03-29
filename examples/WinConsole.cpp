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

#define HTMATCH_WIN_TEST_MODE_AUTO_NOISE       0        // runs as fast as possible with random noise
#define HTMATCH_WIN_TEST_MODE_AUTO_SCALAR      1        // runs as fast as possible with a random number through a Scalar encoder
#define HTMATCH_WIN_TEST_MODE_FULLAUTO         2        // runs as fast as possible with a random valid digit among 64 presets
#define HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT  3        // uses last user keypress as input but keeps iterating on it (64 pre)
#define HTMATCH_WIN_TEST_MODE_STEP_NOISE       4        // draws a new random noisy on user keypress and iterate once
#define HTMATCH_WIN_TEST_MODE_STEP_RAND        5        // draws a new random input on user keypress and iterate once (64 pre)
#define HTMATCH_WIN_TEST_MODE_STEP_SCALAR_RAND 6        // draws a new random number through a Scalar encoder and iterate once
#define HTMATCH_WIN_TEST_MODE_STEP_USER_INPUT  7        // uses user keypress as input and iterate once (64 pre)

// Feel free to define TEST_ITERATION_MODE at will to one of the options above
#define TEST_ITERATION_MODE                    HTMATCH_WIN_TEST_MODE_FULLAUTO

#define HTMATCH_WIN_SCALAR_USE_HASH            1
#define HTMATCH_WIN_NOISE_OVER_256             26       // about 10% noise

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

#define VANILLA_SP_GAUSS_INVBOOST_INHIB

#define VANILLA_SP_SUBNAMESPACE     GaussTest32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_GAUSS_ONLY
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GaussTest16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_GAUSS_ONLY
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GaussTest8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_LOCAL_GAUSS_ONLY
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

/*
#define VANILLA_SP_ALLOW_REROLLS        1

#define VANILLA_SP_SUBNAMESPACE     GlobalTest32
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalTest16
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE

#define VANILLA_SP_SUBNAMESPACE     GlobalTest8
#  define VANILLA_SP_CONFIG           VANILLA_SP_CONFIG_CONST_GLOBAL
#  define VANILLA_SP_SYNAPSE_KIND     VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8
#  include "vanillaHTM/VanillaSPGen.h"
#  include "vanillaHTM/VanillaSPImpl.h"
#  undef VANILLA_SP_CONFIG
#  undef VANILLA_SP_SYNAPSE_KIND
#undef VANILLA_SP_SUBNAMESPACE
*/

// make sure to include WinGDITools (including windows headers) after the vanillaHTM generators...
//   (otherwise we'd have conflict between windows min and max macros and std defs)

#include "examples/SampleTools.h"    
#include "examples/WinGDITools.h"    
#include <unordered_set>

namespace HTMATCH {

    class TestApp {
    public:
        TestApp():
            _singleRand(42u), _uIter(0uLL), _uCurrentCode6b(0u),
            _gnb32(k_uInputSheets), _gnb16(k_uInputSheets), _gnb8(k_uInputSheets),
            _bnb32(k_uInputSheets), _bnb16(k_uInputSheets), _bnb8(k_uInputSheets),
            _lnb32(k_uInputSheets), _lnb16(k_uInputSheets), _lnb8(k_uInputSheets),
            _g32(k_uInputSheets),   _g16(k_uInputSheets),   _g8(k_uInputSheets),
            _b32(k_uInputSheets),   _b16(k_uInputSheets),   _b8(k_uInputSheets),
            _l32(k_uInputSheets),   _l16(k_uInputSheets),   _l8(k_uInputSheets),
            _lnu32(k_uInputSheets), _lnu16(k_uInputSheets), _lnu8(k_uInputSheets),
            _gss32(k_uInputSheets), _gss16(k_uInputSheets), _gss8(k_uInputSheets),
//            _gt32(k_uInputSheets),  _gt16(k_uInputSheets),  _gt8(k_uInputSheets),
            _inputEncoder(), _pInputBuffer(0), _pTmpOutputBuffer(0), _pTmpMinBuffer(0),
            _pTableBitmaps(0), _vecActiveSPcolumns()
        {
            _pInputBuffer = new uint64[k_uQWordPerBinarySheet * k_uInputSheets];
            _pTmpOutputBuffer = new uint64[k_uQWordPerBinarySheet];
            _pTmpMinBuffer = new uint32[VANILLA_HTM_SHEET_2DSIZE];
            _pTableBitmaps = new Gdiplus::Bitmap*[k_uTotalBitmapsCount];
            for (size_t uBitmap = 0u; uBitmap < k_uTotalBitmapsCount; uBitmap++) {
                _pTableBitmaps[uBitmap] = new Gdiplus::Bitmap(VANILLA_HTM_SHEET_WIDTH, VANILLA_HTM_SHEET_HEIGHT, PixelFormat24bppRGB);
            }
            _vecActiveSPcolumns.reserve(64u);
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

        void runIteration()
        {
            const std::vector<uint16>* pVecActiveInputIndices = 0;
            static std::vector<uint16> vecImmediate;
            if (GDI::GDITools::_bCancelInput) {
#if (TEST_ITERATION_MODE > HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT)
                return;
#endif

#if (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_NOISE) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_NOISE)
                // NOOP
#elif (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_SCALAR) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_SCALAR_RAND)
                // NOOP
#else
                _uCurrentCode6b = 64u;
                pVecActiveInputIndices = &(_inputEncoder.getInputVectorEncodingDigitCode(64u));
#endif
            } else {
#if (TEST_ITERATION_MODE > HTMATCH_WIN_TEST_MODE_SPIN_USER_INPUT)
                if (GDI::GDITools::_uLastCharTicks > 0u)
                    return;
                else
                    GDI::GDITools::_uLastCharTicks++;
#endif

#if (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_NOISE) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_NOISE)
                // NOOP
#elif (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_SCALAR) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_SCALAR_RAND)
                // NOOP
#elif (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_FULLAUTO) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_RAND)
                _uCurrentCode6b = uint8(_singleRand.getNext() & 0x003Fu);
#else
                _uCurrentCode6b = _inputEncoder.getDigitCode6bFromChar(unsigned char(GDI::GDITools::_cLastChar));
#endif
            }

#if (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_NOISE) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_NOISE)
            _staticEncodeNoise(_singleRand, vecImmediate);
            SDRTools::toBinaryBitmap64(vecImmediate, _pInputBuffer, VANILLA_HTM_SHEET_BYTES_BINARY * k_uInputSheets);
#elif (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_SCALAR) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_SCALAR_RAND)
            _staticEncodeScalar(_singleRand.getNext(), vecImmediate);
            SDRTools::toBinaryBitmap64(vecImmediate, _pInputBuffer, VANILLA_HTM_SHEET_BYTES_BINARY * k_uInputSheets);
#else
            pVecActiveInputIndices = &(_inputEncoder.getInputVectorEncodingDigitCode(_uCurrentCode6b));
            SDRTools::toBinaryBitmap64(*pVecActiveInputIndices, _pInputBuffer, VANILLA_HTM_SHEET_BYTES_BINARY * k_uInputSheets);
#endif

            _computeUsing(_gnb32,  0u);
            _computeUsing(_gnb16,  1u);
            _computeUsing(_gnb8,   2u);
            _computeUsing(_bnb32,  3u);
            _computeUsing(_bnb16,  4u);
            _computeUsing(_bnb8,   5u);
            _computeUsing(_lnb32,  6u);
            _computeUsing(_lnb16,  7u);
            _computeUsing(_lnb8,   8u);
            _computeUsing(_g32,    9u);
            _computeUsing(_g16,   10u);
            _computeUsing(_g8,    11u);
            _computeUsing(_b32,   12u);
            _computeUsing(_b16,   13u);
            _computeUsing(_b8,    14u);
            _computeUsing(_l32,   15u);
            _computeUsing(_l16,   16u);
            _computeUsing(_l8,    17u);
            _computeUsing(_lnu32, 18u);
            _computeUsing(_lnu16, 19u);
            _computeUsing(_lnu8,  20u);
            _computeUsing(_gss32, 21u);
            _computeUsing(_gss16, 22u);
            _computeUsing(_gss8,  23u);
            /*
            _computeUsing(_gt32,  24u);
            _computeUsing(_gt16,  25u);
            _computeUsing(_gt8,   26u);
            */
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
                        int iStartY = 2 + int(uBitmapPerConf)*35 + 215*int(uConfInCategory);
                        GDI::GDITools::_pGraphics->DrawImage(_pTableBitmaps[uBitmapToDraw], iStartX, iStartY);
                    }
                }
            }

            Gdiplus::FontFamily fontFamily(L"Arial");
            Gdiplus::Font font(&fontFamily, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255u, 0u, 0u, 0u));
            Gdiplus::SolidBrush eraser(Gdiplus::Color(255u, 255u, 255u, 255u));
            GDI::GDITools::_pGraphics->FillRectangle(&eraser, Gdiplus::Rect(5, 200, 60, 100));

            wchar_t szBuf[256];
            std::swprintf(szBuf, 256, L"Iter:%lld", _uIter);
            GDI::GDITools::_pGraphics->DrawString(szBuf, -1, &font, Gdiplus::PointF(5.0f, 200.0f), &textBrush);
            _uIter++;
        }

    private:

        static void _staticEncodeNoise(Rand& noiseGen, std::vector<uint16>& outVecNoise)
        {
            outVecNoise.clear();
            for (uint16 uInput = 0u; uInput < VANILLA_HTM_SHEET_2DSIZE * k_uInputSheets; uInput++) {
                if ((noiseGen.getNext() & 0x000000FFu) < HTMATCH_WIN_NOISE_OVER_256)
                    outVecNoise.push_back(uInput);
            }
        }

        static void _staticEncodeScalar(uint32 uRandomDraw, std::vector<uint16>& outVecEncodedScalar)
        {
            outVecEncodedScalar.clear();
            uRandomDraw = uRandomDraw % 7400;
            for (uint32 uNum = 0u; uNum < 792; uNum++) {
                uint32 uValue = uRandomDraw + uNum;
#ifdef  HTMATCH_WIN_SCALAR_USE_HASH
                uint32 uHashed = 0x9536A9C5u + 374761393u + 4U + (uValue * 3266489917u);
                uHashed = ((uHashed << 17) | (uHashed >> 15)) * 668265263u;
                uHashed ^= uHashed >> 15;
                uHashed *= 2246822519u;
                uHashed ^= uHashed >> 13;
                uHashed *= 3266489917u;
                uHashed ^= uHashed >> 16;
                outVecEncodedScalar.push_back(uint16(uHashed & 0x00001FFFu));
#else
                outVecEncodedScalar.push_back(uint16(uValue));
#endif
            }
        }

        struct DisplayableStats {
            uint16 _uCountActive;           // quite plainly, number of active cells
            uint16 _uCountMissing;          // cells which were previously active for same code and are no more
            uint16 _uCountNew;              // cells which were not previously active for same code and are now
            uint16 _uCountVeryLowActiv;     // number of cells for which average activation is very low relative to expected
            uint16 _uCountVeryHighActiv;    // number of cells for which average activation is very high relative to expected
            float _fAverageActivation;      // Average activation of all cells
            float _fActivationDeviation;    // standard deviation for the distribution with the above average
        };

        template<class SP>
        static void _fillDisplayableStatsWithSpActivationLevels(const SP& sp, DisplayableStats* pStats)
        {
            static const float k_fHalfExpectancy = VANILLA_SP_DEFAULT_DENSITY_RATIO * 0.25f;
            static const float k_fTwiceExpectancy = VANILLA_SP_DEFAULT_DENSITY_RATIO * 4.0f;
            sp.getAverageActivationStats(
                k_fHalfExpectancy, &(pStats->_uCountVeryLowActiv),
                k_fTwiceExpectancy, &(pStats->_uCountVeryHighActiv),
                &(pStats->_fAverageActivation), &(pStats->_fActivationDeviation));
        }
        ; // template termination

        void _fillDisplayableStatsWithResultConsistencyAndUpdateLastResult(size_t uSpIndex, DisplayableStats* pStats)
        {
            pStats->_uCountActive = uint16(_vecActiveSPcolumns.size());
#if (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_NOISE) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_NOISE)
            pStats->_uCountNew = 0u;
            pStats->_uCountMissing = 0u;
#elif (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_AUTO_SCALAR) || (TEST_ITERATION_MODE == HTMATCH_WIN_TEST_MODE_STEP_SCALAR_RAND)
            pStats->_uCountNew = 0u;
            pStats->_uCountMissing = 0u;
#else
            std::unordered_set<uint16> setActiveNow;
            const std::unordered_set<uint16>& prevActive = _prevActivationsPerSp[uSpIndex]._perCode[_uCurrentCode6b];
            pStats->_uCountNew = 0u;
            for (auto itNow = _vecActiveSPcolumns.begin(), itEnd = _vecActiveSPcolumns.end(); itNow != itEnd; itNow++) {
                uint16 uIndex = *itNow;
                setActiveNow.insert(uIndex);
                auto itFound = prevActive.find(uIndex);
                if (itFound == prevActive.end())
                    pStats->_uCountNew++;
            }
            pStats->_uCountMissing = 0u;
            for (auto itPrev = prevActive.begin(), itEnd = prevActive.end(); itPrev != itEnd; itPrev++) {
                uint16 uIndex = *itPrev;
                auto itFound = setActiveNow.find(uIndex);
                if (itFound == setActiveNow.end())
                    pStats->_uCountMissing++;
            }
            _prevActivationsPerSp[uSpIndex]._perCode[_uCurrentCode6b] = setActiveNow;
#endif
        }

        template<class SP>
        void _computeUsing(SP& sp, size_t uSpIndex)
        {
            sp.compute(_pInputBuffer, _vecActiveSPcolumns, true, _pTmpOutputBuffer, _pTmpMinBuffer);
            size_t uBitmapOffset = k_uInputBitmapsCount + uSpIndex * k_uOutputBitmapsPerConf;
            GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 0u], sp.getRawActivationLevels(),
                ColorTools::color1K<uint16>);
            if (sp.doesUseBoosting()) {
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 1u], sp.getBoostingFactors(),
                    //ColorTools::colorScaledBoosting<256u>);
                    //ColorTools::colorScaledBoosting<24u>);
                    //ColorTools::colorBoosting);
                    ColorTools::colorScaledBoosting<40u>);
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 2u], sp.getBoostedActivationLevels(),
                    ColorTools::color256K);
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 3u], _pTmpMinBuffer,
                    ColorTools::color256K);
            } else {
                GDI::GDITools::_toDisplayBitmapFromTypeOnSheet(_pTableBitmaps[uBitmapOffset + 3u], _pTmpMinBuffer,
                    ColorTools::color1K<uint16>);
            }
            GDI::GDITools::_toDisplayBitmapFromBinaryOnSheet(_pTableBitmaps[uBitmapOffset + 4u], _pTmpOutputBuffer,
                255u, 255u, 255u);
            DisplayableStats displayableStats;
            _fillDisplayableStatsWithResultConsistencyAndUpdateLastResult(uSpIndex, &displayableStats);
            _fillDisplayableStatsWithSpActivationLevels(sp, &displayableStats);
            _fillStatBitmap(_pTableBitmaps[uBitmapOffset + 5u], displayableStats);
        }
        ; // template termination

        static void _displayNegPosValues(uint8* pBuffer, u16fast uNegative, u16fast uPositive, u16fast uPosY)
        {
            if (uNegative >= 1u)
                _displaySlot(pBuffer, 28u, 2u, uPosY, 255u, 255u, 0u);
            if (uNegative >= 2u)
                _displaySlot(pBuffer, 25u, 2u, uPosY, 255u, 191u, 0u);
            if (uNegative >= 3u)
                _displaySlot(pBuffer, 22u, 2u, uPosY, 255u, 127u, 0u);
            if (uNegative >= 4u)
                _displaySlot(pBuffer, 19u, 2u, uPosY, 255u, 63u, 0u);
            if (uNegative >= 5u)
                _displaySlot(pBuffer, 16u, 2u, uPosY, 255u, 63u, 0u);
            if (uNegative >= 6u) {
                u16fast uWidth = min(uNegative-5u, 14u);
                _displaySlot(pBuffer, 15u - uWidth, uWidth, uPosY, 255u, 0u, 0u);
            }
        
            if (uPositive >= 1u)
                _displaySlot(pBuffer, 34u, 2u, uPosY, 255u, 255u, 0u);
            if (uPositive >= 2u)
                _displaySlot(pBuffer, 37u, 2u, uPosY, 255u, 191u, 0u);
            if (uPositive >= 3u)
                _displaySlot(pBuffer, 40u, 2u, uPosY, 255u, 127u, 0u);
            if (uPositive >= 4u)
                _displaySlot(pBuffer, 43u, 2u, uPosY, 255u, 63u, 0u);
            if (uPositive >= 5u)
                _displaySlot(pBuffer, 46u, 2u, uPosY, 255u, 63u, 0u);
            if (uPositive >= 6u) {
                u16fast uWidth = min(uPositive-5u, 14u);
                _displaySlot(pBuffer, 49u, uWidth, uPosY, 255u, 0u, 0u);
            }
        }

        static void _fillStatBitmap(Gdiplus::Bitmap* pBitmap, const DisplayableStats& stats)
        {
            uint8* pBuffer = GDI::GDITools::getZeroedByteBufferForSheetBitmap(pBitmap);
            if (pBuffer) {
                for (u16fast uStatZone = 0u; uStatZone < 4u; uStatZone++) {
                    for (u16fast uVerticalBar = 1u; uVerticalBar < 7u; uVerticalBar++) {
                        u16fast uPosY = uStatZone * 8u + uVerticalBar;
                        u16fast uPosX = 31;
                        u16fast uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + uPosX) * 3u;
                        pBuffer[uOffset + 0u] = 255u;
                        pBuffer[uOffset + 1u] = 255u;
                        pBuffer[uOffset + 2u] = 255u;
                        uPosX = 32;
                        uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + uPosX) * 3u;
                        pBuffer[uOffset + 0u] = 255u;
                        pBuffer[uOffset + 1u] = 255u;
                        pBuffer[uOffset + 2u] = 255u;
                    }
                }

                _displayNegPosValues(pBuffer, stats._uCountMissing, stats._uCountNew, 1u);
                int16 iCountDiff41 = int16(stats._uCountActive) - 41;
                _displayNegPosValues(pBuffer, iCountDiff41<0?u16fast(-iCountDiff41):0u,
                    iCountDiff41>0?u16fast(iCountDiff41):0u, 9u);
                _displayNegPosValues(pBuffer, stats._uCountVeryLowActiv, stats._uCountVeryHighActiv, 17u);
                float fAvgWithFloor = max(VANILLA_SP_DEFAULT_DENSITY_RATIO/100.0f, stats._fAverageActivation);
                float fLogDiffFromExpMean = std::log(fAvgWithFloor / VANILLA_SP_DEFAULT_DENSITY_RATIO);
                i16fast iLogDiffFromExpMean = i16fast(std::round(fLogDiffFromExpMean * 20.0f));

                if (iLogDiffFromExpMean < 0) {
                    for (i16fast iBackwards = -1; iBackwards >= max(-30, iLogDiffFromExpMean); iBackwards--) {
                        i16fast iPosX = 31+iBackwards;
                        for (u16fast uPosY = 25u; uPosY < 28u; uPosY++) {
                            u16fast uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + u16fast(iPosX)) * 3u;
                            pBuffer[uOffset + 0u] = 255u;
                            pBuffer[uOffset + 1u] = 255u - (uint16(-iBackwards) * 4u);
                            pBuffer[uOffset + 2u] = 0u;
                        }
                    }
                } else if (iLogDiffFromExpMean > 0) {
                    for (i16fast iForward = 1; iForward <= min(30, iLogDiffFromExpMean); iForward++) {
                        i16fast iPosX = 32+iForward;
                        for (u16fast uPosY = 25u; uPosY < 28u; uPosY++) {
                            u16fast uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + u16fast(iPosX)) * 3u;
                            pBuffer[uOffset + 0u] = 255u;
                            pBuffer[uOffset + 1u] = 255u - (uint16(iForward) * 4u);
                            pBuffer[uOffset + 2u] = 0u;
                        }
                    }
                }

                float fLogDev = std::log(1.0f + stats._fActivationDeviation);
                u16fast uLogDev = u16fast(std::round(fLogDev * 300.0f));
                for (u16fast uDev = 0u; uDev < min(uLogDev, 31u); uDev++) {
                    for (u16fast uPosY = 28u; uPosY < 31u; uPosY++) {
                        u16fast uPosX = 31 - uDev;
                        u16fast uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + uPosX) * 3u;
                        pBuffer[uOffset + 0u] = 0u;
                        pBuffer[uOffset + 1u] = 192u - (uDev * 6u);
                        pBuffer[uOffset + 2u] = 255u;
                        uPosX = 32 + uDev;
                        uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + uPosX) * 3u;
                        pBuffer[uOffset + 0u] = 0u;
                        pBuffer[uOffset + 1u] = 192u - (uDev * 6u);
                        pBuffer[uOffset + 2u] = 255u;
                    }
                }

                GDI::GDITools::whenDoneWithByteBufferOver(pBitmap);
                pBuffer = 0;
            }
        }

        static void _displaySlot(uint8* pBuffer, u16fast uStartX, u16fast uSizeX, u16fast uStartY, uint8 uRed, uint8 uGreen, uint8 uBlue) {
            for (u16fast uX = 0u; uX < uSizeX; uX++) {
                u16fast uPosX = uStartX + uX;
                for (u16fast uY = 0u; uY < 6u; uY++) {
                    u16fast uPosY = uStartY + uY;
                    u16fast uOffset = ((uPosY * VANILLA_HTM_SHEET_WIDTH) + uPosX) * 3u;
                    pBuffer[uOffset + 0u] = uBlue;
                    pBuffer[uOffset + 1u] = uGreen;
                    pBuffer[uOffset + 2u] = uRed;
                }
            }
        }

        Rand _singleRand;
        uint64 _uIter;
        uint8 _uCurrentCode6b;

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

        GaussTest32::VanillaSP _gss32;
        GaussTest16::VanillaSP _gss16;
        GaussTest8::VanillaSP _gss8;

        /*
        GlobalTest32::VanillaSP _gt32;
        GlobalTest16::VanillaSP _gt16;
        GlobalTest8::VanillaSP _gt8;
        */

        FixedDigitEncoder _inputEncoder;
        uint64* _pInputBuffer;
        uint64* _pTmpOutputBuffer;
        uint32* _pTmpMinBuffer;
        Gdiplus::Bitmap** _pTableBitmaps;
        std::vector<uint16> _vecActiveSPcolumns;

        static const size_t k_uQWordPerBinarySheet = VANILLA_HTM_SHEET_2DSIZE >> 6u;
        static const size_t k_uInputSheets = 4u;
        static const size_t k_uInputBitmapsCount = k_uInputSheets;
        static const size_t k_uOutputBitmapsPerConf = 6u;
        static const size_t k_uOutputBitConfPerConfCategory = 3u;
        static const size_t k_uOutputConfCategories = 8u;
        static const size_t k_uOutputBitmapCount = k_uOutputBitmapsPerConf * k_uOutputBitConfPerConfCategory * k_uOutputConfCategories;
        static const size_t k_uTotalBitmapsCount = k_uInputBitmapsCount + k_uOutputBitmapCount;

        struct PrevActivationForSp {
            std::unordered_set<uint16> _perCode[65u];
        };
        PrevActivationForSp _prevActivationsPerSp[k_uOutputConfCategories * k_uOutputConfCategories];
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

