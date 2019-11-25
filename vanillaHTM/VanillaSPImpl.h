/* -----------------------------------
 * HTMATCH
 * VanillaSPImpl.h
 * -----------------------------------
 * Implements the vanilla Spatial Pooler
 * Warning : No multi-inclusion guard !!!
 *   This file is intended to be included multiple times with different config options indeed, if the user wishes so.
 *      (to have multiple declared versions, you may #define VANILLA_SP_SUBNAMESPACE to an identifier of your choice)
 * Note: do not forget to #include "VanillaSPGen.h" with same SP config parameters before including this one.
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

// - - - - - - - - - - - - - - - - - - - -
// The following defines and includes are simply to help IDEs to parse this, even if taken in isolation from a calling site.
//   Since nominally, your project should compile this file "included" from some place where the following '#ifndef' won't
//   trigger (also, feel free to change VANILLA_SP_CONFIG and VANILLA_SP_SYNAPSE_KIND values below to enlight different parts
//   of the code if you have such IDE)
// - - - - - - - - - - - - - - - - - - - -
#ifndef _VANILLA_SP_GEN_H
#  include "VanillaHTMConfig.h"
#  ifndef VANILLA_SP_CONFIG
#    define VANILLA_SP_CONFIG            VANILLA_SP_CONFIG_CONST_LOCAL_GAUSS_ONLY
#  endif
#  ifndef VANILLA_SP_SYNAPSE_KIND
#    define VANILLA_SP_SYNAPSE_KIND      VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16
#  endif
#  ifndef VANILLA_SP_SUBNAMESPACE
#    define VANILLA_SP_SUBNAMESPACE      Default_
#  endif
#  include "VanillaSPGen.h"
#endif

#include "tools/system.h"
#include "tools/rand.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace HTMATCH {
#if defined(VANILLA_SP_SUBNAMESPACE)
    namespace VANILLA_SP_SUBNAMESPACE {
#endif

// - - - - - - - - - - - - - - - - - - - -
// Updates a floating-point buffer of moving-averages used in vanilla SP across all columns in the sheet,
//   integrating a new binary value, over 'IntegrationWindow' runs
//   (ie, new value is weighted 1/uIntegrationWindow against previous avg)
// - - - - - - - - - - - - - - - - - - - -
static void _integrateBinaryFieldToMovingAverages(float* pColMajorMovingAverages, const uint64* pBinaryBitmap,
    uint64 uIntegrationWindow)
{
    const float fIntegrationWindow = float(uIntegrationWindow);
    const float fInvWindow = 1.0f / fIntegrationWindow;
    const float fWindowMinusOne = fIntegrationWindow - 1.0f;
    float *pCurrentVal = pColMajorMovingAverages;
    for (size_t uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE; uIndex++, pCurrentVal++) {
        size_t uQword = uIndex >> 6u;
        size_t uBit = uIndex & 0x003Fu;
        float fValueNow = float((pBinaryBitmap[uQword] >> uBit) & 1uLL);
        float fPrevValue = *pCurrentVal;
        *pCurrentVal = (fPrevValue * fWindowMinusOne + fValueNow) * fInvWindow;
    }
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and signed delta to apply
// Will be implemented differently based on the current 'VANILLA_SP_SYNAPSE_KIND' option
// - - - - - - - - - - - - - - - - - - - -
static VANILLA_SP_SYN_PERM_TYPE _updatePermanence(VANILLA_SP_SYN_PERM_TYPE previousPermanence,
    VANILLA_SP_SYN_SIGNED_PERM_TYPE permanenceDeltaToApply)
{
#if (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32)
    // VANILLA_SP_SYN_PERM_TYPE and VANILLA_SP_SYN_SIGNED_PERM_TYPE as floats
    return std::max(0.0f, std::min(1.0f, previousPermanence + permanenceDeltaToApply));
#else
    // VANILLA_SP_SYN_PERM_TYPE as either uint16 or uint8 (bounded high to VANILLA_SP_SYN_PERM_TYPE_MAX),
    //   VANILLA_SP_SYN_SIGNED_PERM_TYPE as int32
    int32 iPrevious = int32(previousPermanence);
    int32 iNow = iPrevious + permanenceDeltaToApply;
    iNow = unbranchingClampLowToZero(iNow);
    iNow = unbranchingClampHighTo(VANILLA_SP_SYN_PERM_TYPE_MAX, iNow);
    return VANILLA_SP_SYN_PERM_TYPE(iNow);
#endif
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and unsigned increase to apply
// Will be implemented differently based on the current 'VANILLA_SP_SYNAPSE_KIND' option
// - - - - - - - - - - - - - - - - - - - -
static VANILLA_SP_SYN_PERM_TYPE _increasePermanence(VANILLA_SP_SYN_PERM_TYPE previousPermanence,
    VANILLA_SP_SYN_PERM_TYPE unsignedPermanenceIncrease)
{
#if (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32)
    // VANILLA_SP_SYN_PERM_TYPE and VANILLA_SP_SYN_SIGNED_PERM_TYPE as floats
    return std::min(1.0f, previousPermanence + unsignedPermanenceIncrease);
#else
    // VANILLA_SP_SYN_PERM_TYPE as either uint16 or uint8 (bounded high to VANILLA_SP_SYN_PERM_TYPE_MAX),
    //   VANILLA_SP_SYN_SIGNED_PERM_TYPE as int32
    int32 iPrevious = int32(previousPermanence);
    int32 iNow = iPrevious + int32(unsignedPermanenceIncrease);
    iNow = unbranchingClampHighTo(VANILLA_SP_SYN_PERM_TYPE_MAX, iNow);
    return VANILLA_SP_SYN_PERM_TYPE(iNow);
#endif
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and unsigned decrease to apply
// Will be implemented differently based on the current 'VANILLA_SP_SYNAPSE_KIND' option
// - - - - - - - - - - - - - - - - - - - -
static VANILLA_SP_SYN_PERM_TYPE _decreasePermanence(VANILLA_SP_SYN_PERM_TYPE previousPermanence,
    VANILLA_SP_SYN_PERM_TYPE unsignedPermanenceDecrease)
{
#if (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32)
    // VANILLA_SP_SYN_PERM_TYPE and VANILLA_SP_SYN_SIGNED_PERM_TYPE as floats
    return std::max(0.0f, previousPermanence - unsignedPermanenceDecrease);
#else
    // VANILLA_SP_SYN_PERM_TYPE as either uint16 or uint8 (bounded high to VANILLA_SP_SYN_PERM_TYPE_MAX),
    //   VANILLA_SP_SYN_SIGNED_PERM_TYPE as int32
    int32 iPrevious = int32(previousPermanence);
    int32 iNow = iPrevious - int32(unsignedPermanenceDecrease);
    iNow = unbranchingClampLowToZero(iNow);
    return VANILLA_SP_SYN_PERM_TYPE(iNow);
#endif
}

// - - - - - - - - - - - - - - - - - - - -
// Helper method for implementing _getBestFromRange() below
// - - - - - - - - - - - - - - - - - - - -
static u16fast _insertInQueue(uint32* pQueue, uint32 uValueToInsert, u16fast uInsertPos, u16fast uQueueCapacity, u16fast uQueueSize, uint32* ioCurrentTailValue)
{
    if (uQueueSize < uQueueCapacity) {
        for (u16fast uIndexAfter = uQueueSize; uIndexAfter > uInsertPos; uIndexAfter--) {
            pQueue[uIndexAfter] = pQueue[uIndexAfter-1u];
        }
        uQueueSize++;
        pQueue[uInsertPos] = uValueToInsert;
    } else {
        for (u16fast uIndexAfter = uQueueCapacity-1u; uIndexAfter > uInsertPos; uIndexAfter--) {
            pQueue[uIndexAfter] = pQueue[uIndexAfter-1u];
        }
        pQueue[uInsertPos] = uValueToInsert;
        *ioCurrentTailValue = pQueue[uQueueCapacity-1u];
    }
    return uQueueSize;
}

// - - - - - - - - - - - - - - - - - - - -
// Fills a table of 'uWinnerK+1' best values found in a rectangular region on the cortical sheet,
//   typically from either Raw or Boosted 'ActivationLevel' values. Raw levels are typically 16b, and boosted levels are
//     typically 32b, but algorithm is same between the two (thus specifying 'ActivationLevelType' template parameter)
//   bCareForXWrap must be specified true whenever uStartX+uSizeX > VANILLA_SP_SHEET_WIDTH,
//   bCareForYWrap must be specified true whenever uStartY+uSizeY > VANILLA_SP_SHEET_HEIGHT,
// However, chosing between them is left to user discretion as various algorithms are able to ensure they won't get out of bounds
//   for one or the other beforehand, and them being template parameters will allow an overhead-free conditional implementation.
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bCareForXWrap, bool bCareForYWrap>
static u16fast _getBestFromRange(u16fast uStartX, u16fast uSizeX, u16fast uStartY, u16fast uSizeY,
    const ActivationLevelType* pColMajorActivationLevels, uint32* pQueueOfBestValues, u16fast uQueueCapacity)
{
    uint32 uLowestIn = 0u;
    if (VANILLA_SP_DEFAULT_STIMULUS_THRESHOLD > 0) {
        ActivationLevelType uStimThresholdPossiblyBoosted = ActivationLevelType(VANILLA_SP_DEFAULT_STIMULUS_THRESHOLD);
#ifdef VANILLA_SP_SCALE_THRESHOLD_WHEN_BOOSTED
        // Detecting (at compile time) whether ActivationLevelType is on 4 bytes (=> uint32, indicative of a 'boosted' input)
        if (sizeof(ActivationLevelType) == 4u)      // Boosting uses an equivalent of fix point, 8b after point
            uStimThresholdPossiblyBoosted <<= 8u;   //  => we also shift threshold value by 8b, and we're all set for this issue!
#endif
        uLowestIn = uStimThresholdPossiblyBoosted - 1u;
    }
    // inserts one less-than activation threshold as tail value.
    pQueueOfBestValues[0] = uLowestIn;
    u16fast uQueueSize = 1u;
    i16fast iEndX = i16fast(uStartX)+i16fast(uSizeX);
    i16fast iEndY = i16fast(uStartY)+i16fast(uSizeY);
    if (bCareForXWrap) iEndX &= VANILLA_HTM_SHEET_XMASK;
    if (bCareForYWrap) iEndY &= VANILLA_HTM_SHEET_YMASK;  
    for (i16fast iX = i16fast(uStartX); iX != iEndX; iX = bCareForXWrap ? ((iX+1) & VANILLA_HTM_SHEET_XMASK) : iX+1) {
        for (i16fast iY = i16fast(uStartY); iY != iEndY; iY = bCareForYWrap ? ((iY+1) & VANILLA_HTM_SHEET_YMASK) : iY+1) {
            u16fast uIndex = u16fast((iX << VANILLA_HTM_SHEET_SHIFT_DIVY) + iY);
            ActivationLevelType uActivationLevel = pColMajorActivationLevels[uIndex];
            // We only need to consider inserting that value if we're greater than the know tail of the queue
            if (uActivationLevel > uLowestIn) { // (will be at least the activation threshold if queue is not yet full)
                bool bInserted = false;
                // reverse-iterating the queue to find a correct insert position (we want to insert keeping the queue sorted)
                for (u16fast uIndexIn = uQueueSize-1u; uIndexIn; uIndexIn--) {
                    // checking value just before to check if we're greater 
                    if (pQueueOfBestValues[uIndexIn-1] >= uActivationLevel) { // found greater predecessor => insert right there
                        uQueueSize = _insertInQueue(pQueueOfBestValues, uActivationLevel, uIndexIn, uQueueCapacity, uQueueSize, &uLowestIn);
                        bInserted = true;
                        break;
                    }
                }
                // If we did not find a greater predecessor checking the full queue, it means we should insert to head of queue
                if (!bInserted) {
                    uQueueSize = _insertInQueue(pQueueOfBestValues, uActivationLevel, 0u, uQueueCapacity, uQueueSize, &uLowestIn);
                }
            }
        }
    }
    return uQueueSize-1u;
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// Returns a sum of all floating-point values over a rectangular region on the cortical sheet.
//   bCareForXWrap must be specified true whenever uStartX+uSizeX > VANILLA_SP_SHEET_WIDTH,
//   bCareForYWrap must be specified true whenever uStartY+uSizeY > VANILLA_SP_SHEET_HEIGHT,
// However, chosing between them is left to user discretion as various algorithms are able to ensure they won't get out of bounds
//   for one or the other beforehand, and them being template parameters will allow an overhead-free conditional implementation.
// - - - - - - - - - - - - - - - - - - - -
template<bool bCareForXWrap, bool bCareForYWrap>
static float _getSumFromRange(u16fast uStartX, u16fast uSizeX, u16fast uStartY, u16fast uSizeY, const float* pColumnMajorFloatValues) {
    float fResult = 0.0f;
    i16fast iEndX = i16fast(uStartX)+i16fast(uSizeX);
    i16fast iEndY = i16fast(uStartY)+i16fast(uSizeY);
    if (bCareForXWrap) iEndX &= VANILLA_HTM_SHEET_XMASK;
    if (bCareForYWrap) iEndY &= VANILLA_HTM_SHEET_YMASK;
    for (i16fast iX = i16fast(uStartX); iX != iEndX; iX = bCareForXWrap ? ((iX+1) & VANILLA_HTM_SHEET_XMASK) : iX+1) {
        if (bCareForYWrap) {
            for (i16fast iY = i16fast(uStartY); iY != iEndY; iY = (iY+1) & VANILLA_HTM_SHEET_YMASK) {
                fResult += pColumnMajorFloatValues[(iX << VANILLA_HTM_SHEET_SHIFT_DIVY) + iY];
            }
        } else {
            u16fast uIndex = u16fast(iX) << VANILLA_HTM_SHEET_SHIFT_DIVY;
            uIndex += uStartY;
            const float* pStartFloatValue = pColumnMajorFloatValues + uIndex;
            for (const float *pCurrent = pStartFloatValue, *pEnd = pStartFloatValue + uSizeY; pCurrent < pEnd; pCurrent++) {
                fResult += *pCurrent;
            }
        }
    }
    return fResult;
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// Returns max found from all floating-point values over a rectangular region on the cortical sheet.
//   bCareForXWrap must be specified true whenever uStartX+uSizeX > VANILLA_SP_SHEET_WIDTH,
//   bCareForYWrap must be specified true whenever uStartY+uSizeY > VANILLA_SP_SHEET_HEIGHT,
// However, chosing between them is left to user discretion as various algorithms are able to ensure they won't get out of bounds
//   for one or the other beforehand, and them being template parameters will allow an overhead-free conditional implementation.
// - - - - - - - - - - - - - - - - - - - -
template<bool bCareForXWrap, bool bCareForYWrap>
static float _getMaxFromRange(u16fast uStartX, u16fast uSizeX, u16fast uStartY, u16fast uSizeY, const float* pColumnMajorFloatValues) {
    float fMaxFound = 0.0f;
    i16fast iEndX = i16fast(uStartX)+i16fast(uSizeX);
    i16fast iEndY = i16fast(uStartY)+i16fast(uSizeY);
    if (bCareForXWrap) iEndX &= VANILLA_HTM_SHEET_XMASK;
    if (bCareForYWrap) iEndY &= VANILLA_HTM_SHEET_YMASK;
    for (i16fast iX = i16fast(uStartX); iX != iEndX; iX = bCareForXWrap ? ((iX+1) & VANILLA_HTM_SHEET_XMASK) : iX+1) {
        if (bCareForYWrap) {
            for (i16fast iY = i16fast(uStartY); iY != iEndY; iY = (iY+1) & VANILLA_HTM_SHEET_YMASK) {
                float fThere = pColumnMajorFloatValues[(iX << VANILLA_HTM_SHEET_SHIFT_DIVY) + iY];
                if (fThere > fMaxFound)
                    fMaxFound = fThere;
            }
        } else {
            u16fast uIndex = u16fast(iX) << VANILLA_HTM_SHEET_SHIFT_DIVY;
            uIndex += uStartY;
            const float* pStartFloatValue = pColumnMajorFloatValues + uIndex;
            for (const float *pCurrent = pStartFloatValue, *pEnd = pStartFloatValue + uSizeY; pCurrent < pEnd; pCurrent++) {
                float fThere = *pCurrent;
                if (fThere > fMaxFound)
                    fMaxFound = fThere;
            }
        }
    }
    return fMaxFound;
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// One of the helper-methods for implementing a very-optimized sum or max filter over a rectangular kernel.
// @see _computeOptiForSum, _computeOptiForMax, _computeOptiForBest
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType, typename OutType, typename InitFunc, typename IntegrationFunc>
static void _computeRowMajorGFromColMajorValues(const ValType* pColumnValues, OutType* outG, InitFunc initializer,
    IntegrationFunc integrator, u8fast uRadius, u8fast uKernelSize, u8fast uKernelCountY, u8fast uRemainderY,
    u8fast uAfterRemainderY, u8fast uLastKernelSize)
{
    OutType integratedVal;
    const ValType* pInput = pColumnValues;
    // start new integration for first kernel
    initializer(integratedVal);
    u8fast uRelY = 0u;
    // integrate values without emitting output, for the first "uRadius" count of them
    for (; uRelY < uRadius; uRelY++, pInput++) {
        integrator(integratedVal, *pInput);
    }
    // integrate values and emit output for the remaining positions on first kernel
    // => output will start offset by +uRadius
    for (; uRelY < uKernelSize; uRelY++, pInput++, outG += VANILLA_HTM_SHEET_WIDTH) {
        integrator(integratedVal, *pInput);
        *outG = integratedVal;
    }
    // iterate all kernels ending before reaching sheet height
    for (u8fast uK = 1u; uK < uKernelCountY; uK++) {
        // start new integration for this kernel
        initializer(integratedVal);
        // then integrate values and emit output
        for (uRelY = 0u; uRelY < uKernelSize; uRelY++, pInput++, outG += VANILLA_HTM_SHEET_WIDTH) {
            integrator(integratedVal, *pInput);
            *outG = integratedVal;
        }
    }
    // start new integration for one kernel after that
    initializer(integratedVal);
    // integrate values and emit output for the remaining positions until sheet height
    for (uRelY = 0u; uRelY < uRemainderY; uRelY++, pInput++, outG += VANILLA_HTM_SHEET_WIDTH) {
        integrator(integratedVal, *pInput);
        *outG = integratedVal;
    }
    // Wraps around for the remaining positions on same kernel, if need be
    pInput = pColumnValues;
    // integrate values and emit output for the remaining positions of the first wrapped kernel
    for (uRelY = 0u; uRelY < uAfterRemainderY; uRelY++, pInput++, outG += VANILLA_HTM_SHEET_WIDTH) {
        integrator(integratedVal, *pInput);
        *outG = integratedVal;
    }
    // Starts another kernel after that, if need be to reach sheet height + uRadius (still wrapped)
    initializer(integratedVal);
    // integrate values and emit output for all positions until uRadius count of wrapped Y have been visited
    for (uRelY = 0u; uRelY < uLastKernelSize; uRelY++, pInput++, outG += VANILLA_HTM_SHEET_WIDTH) {
        integrator(integratedVal, *pInput);
        *outG = integratedVal;
    }
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// One of the helper-methods for implementing a very-optimized sum or max filter over a rectangular kernel.
// @see _computeSumFromOpti, _computeMaxFromOpti, _computeBestFromOpti
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType, typename OutType, typename InitFunc, typename IntegrationFunc>
static void _computeRowMajorRevHFromColMajorValues(const ValType* pColumnValues, OutType* outRevH, InitFunc initializer,
    IntegrationFunc integrator, u8fast uRadius, u8fast uKernelSize, u8fast uKernelCount, u8fast uRemainderY,
    u8fast uAfterRemainderY, u8fast uLastKernelSize)
{



    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// One of the helper-methods for implementing a very-optimized sum or max filter over a rectangular kernel.
// @see computeSumFromOpti, computeMaxFromOpti, computeBestFromOpti
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType, typename OutType, typename IntegrationFunc>
static void _computeColMajorGFromRowMajorGandH(const ValType* pRowGs, const ValType* pRowRevHs, OutType* outG, OutType startVal,
    IntegrationFunc integrator, u8fast uRadius, u8fast uKernelSize, u8fast uKernelCount, u8fast uRemainderY,
    u8fast uAfterRemainderY, u8fast uLastKernelSize)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// One of the helper-methods for implementing a very-optimized sum or max filter over a rectangular kernel.
// @see computeSumFromOpti, computeMaxFromOpti, computeBestFromOpti
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType, typename OutType, typename IntegrationFunc>
static void _computeColMajorRevHFromRowMajorGandH(const ValType* pRowGs, const ValType* pRowRevHs, OutType* outRevH,
    OutType startVal, IntegrationFunc integrator, u8fast uRadius, u8fast uKernelSize, u8fast uKernelCount, u8fast uRemainderY,
    u8fast uAfterRemainderY, u8fast uLastKernelSize)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
// Searching for 'Max' in a neighborhood was re-adapted from the technique expalined in:
// @see "A fast algorithm for local minimum and maximum filters on rectangular and octagonal kernels" by Marcel van Herk
// - - - - - - - - - - - - - - - - - - - -
// Adaptations to HTM SpatialPooler requirements were also found for sum, K-best, and One-best !
// - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - -
// Optimized sum across neighborhood
// - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValTypeSum>
static ValTypeSum _computeSumFromOpti(const ValTypeSum* pCurrentColMajSumXG, const ValTypeSum* pCurrentColMajSumXRevH,
    u8fast uRadius)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValTypeIn, typename ValTypeSum>
static void _computeOptiForSumY(const ValTypeIn* pColMajValues, u8fast uRadius,
    ValTypeSum* pRowMajSumYG, ValTypeSum* pRowMajSumYRevH)
{
    u8fast uKernelSize = uRadius * 2u + 1u;
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValTypeSum>
static void _computeOptiForSumX(const ValTypeSum* pRowMajSumYG, const ValTypeSum* pRowMajSumYRevH, u8fast uRadius,
    ValTypeSum* pColMajSumXG, ValTypeSum* pColMajSumXRevH)
{
    u8fast uKernelSize = uRadius * 2u + 1u;
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// Optimized max across neighborhood
// - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static ValType _computeMaxFromOpti(const ValType* pCurrentColMajMaxXG, const ValType* pCurrentColMajMaxXRevH, u8fast uRadius)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForMaxY(const ValType* pColMajValues, u8fast uRadius, ValType* pRowMajMaxYG, ValType* pRowMajMaxYRevH)
{
    u8fast uKernelSize = uRadius * 2u + 1u;
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForMaxX(const ValType* pRowMajMaxYG, const ValType* pRowMajMaxYRevH, u8fast uRadius,
    ValType* pColMajMaxXG, ValType* pColMajMaxXRevH)
{
    u8fast uKernelSize = uRadius * 2u + 1u;
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// Optimized k-best across neighborhood
// - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static size_t _computeKBestFromOpti(const ValType* pCurrentColMajBestXG, const ValType* pCurrentColMajBestXRevH, u8fast uRadius,
    u8fast uMaxK)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForKBestY(const ValType* pColMajValues, u8fast uRadius,
    ValType* pRowMajBestYG, ValType* pRowMajBestYRevH, u8fast uMaxK)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForKBestX(const ValType* pRowMajBestYG, const ValType* pRowMajBestYRevH, u8fast uRadius,
    ValType* pColMajBestXG, ValType* pColMajBestXRevH, u8fast uMaxK)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// Optimized one-best across neighborhood
// - - - - - - - - - - - - - - - - - - - -

template<typename ValType>
struct BestStruct {
    ValType _uBestValue;
    uint16  _uBestIndex;
};

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static BestStruct<ValType> _computeSingleBestFromOpti(const BestStruct<ValType>* pCurrentColMajBestXG,
    const BestStruct<ValType>* pCurrentColMajBestXRevH, u8fast uRadius)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForSingleBestY(const ValType* pColMajValues, u8fast uRadius,
    BestStruct<ValType>* pRowMajBestYG, BestStruct<ValType>* pRowMajBestYRevH)
{
    // TODO
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ValType>
static void _computeOptiForSingleBestX(const BestStruct<ValType>* pRowMajBestYG, const BestStruct<ValType>* pRowMajBestYRevH,
    u8fast uRadius, BestStruct<ValType>* pColMajBestXG, BestStruct<ValType>* pColMajBestXRevH)
{
    // TODO
}
; // template termination

/*
static void computeOptiForMaxY(const float* pColumnMajorFloatValues, size_t uRadius, float* pRowMajorMaxY_G, float* pRowMajorMaxY_H) {
size_t uKernelSize = uRadius * 2u + 1u;
size_t uKernelCountY = DEFAULT_SHEET_HEIGHT / uKernelSize;
size_t uRemainderY = DEFAULT_SHEET_HEIGHT % uKernelSize;
size_t uAfterRemainderY = uKernelSize - uRemainderY;
size_t uLastKernelSize = 0u;
if (uAfterRemainderY >= uRadius) {
uAfterRemainderY = uRadius;
} else {
uLastKernelSize = uRadius - uAfterRemainderY;
}
const float* pInputG = pColumnMajorFloatValues;
for (size_t uX = 0u; uX < DEFAULT_SHEET_WIDTH; uX++) {
float fMax = -FLT_MAX;
for (size_t uRelY = 0u; uRelY < uRadius; uRelY++, pInputG++) {
float fInput = *pInputG;
if (fMax < fInput)
fMax = fInput;
}
float* pOutputG = pRowMajorMaxY_G + uX;
for (size_t uRelY = uRadius; uRelY < uKernelSize; uRelY++, pInputG++) {
float fInput = *pInputG;
if (fMax < fInput)
fMax = fInput;
*pOutputG = fMax;
}
for (size_t uK = 1u; uK < uKernelCountY; uK++) {
float fMax = -FLT_MAX;
for (size_t uRelY = 0u; uRelY < uKernelSize; uRelY++, pInputG++, pOutputG+=DEFAULT_SHEET_WIDTH) {
float fInput = *pInputG;
if (fMax < fInput)
fMax = fInput;
*pOutputG = fMax;
}
}
{
float fMax = -FLT_MAX;
for (size_t uRelY = 0u; uRelY < uRemainderY; uRelY++, pInputG++, pOutputG+=DEFAULT_SHEET_WIDTH) {
float fInput = *pInputG;
if (fMax < fInput)
fMax = fInput;
*pOutputG = fMax;
}
const float* pWrapedStartInputG = pColumnMajorFloatValues + DEFAULT_SHEET_HEIGHT * uX;
for (size_t uRelY = 0u; uRelY < uAfterRemainderY; uRelY++, pWrapedStartInputG++, pOutputG+=DEFAULT_SHEET_WIDTH) {
float fInput = *pWrapedStartInputG;
if (fMax < fInput)
fMax = fInput;
*pOutputG = fMax;
}
fMax = -FLT_MAX;
for (size_t uRelY = 0u; uRelY < uLastKernelSize; uRelY++, pWrapedStartInputG++, pOutputG+=DEFAULT_SHEET_WIDTH) {
float fInput = *pWrapedStartInputG;
if (fMax < fInput)
fMax = fInput;
*pOutputG = fMax;
}
}
}

}
*/

// - - - - - - - - - - - - - - - - - - - -
// Trying to behave as-was-intended
// In particular, correctly handles wrapping concerns.
// - - - - - - - - - - - - - - - - - - - -
static float _computeCorrectedAvgConnectedSpanFor(u16fast uX, u16fast uY, u16fast uZCountInput, const VanillaSP::Segment& segment)
{
    if (uZCountInput > 1u) {
        // if input has more than one sheet, we're considered '3D'
        u16fast uHalfCountZ = uZCountInput >> 1u;
        u16fast uMaxDiffX = 0u;
        u16fast uMaxDiffY = 0u;
        u16fast uCount = segment._uCount;
        const uint16* pPreSyn = segment._tPreSynIndex;
        const VANILLA_SP_SYN_PERM_TYPE* pPerm = segment._tPermValue;
        for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
            if (*pPerm >= VANILLA_SP_SYN_CONNECTED_PERM) {
                u16fast uPreSynCellIndex = *pPreSyn;
                u16fast uPreSynCellY = uPreSynCellIndex & VANILLA_HTM_SHEET_YMASK;
                u16fast uPreSynCellX = (uPreSynCellIndex >> VANILLA_HTM_SHEET_SHIFT_DIVY) & VANILLA_HTM_SHEET_2DMASK;
                u16fast uDiffX = wrappedDistanceBetween(uPreSynCellX, uX, VANILLA_HTM_SHEET_XMASK, VANILLA_HTM_SHEET_SHIFT_DIVX);
                u16fast uDiffY = wrappedDistanceBetween(uPreSynCellY, uY, VANILLA_HTM_SHEET_YMASK, VANILLA_HTM_SHEET_SHIFT_DIVY);
                if (uDiffX > uMaxDiffX)
                    uMaxDiffX = uDiffX;
                if (uDiffY > uMaxDiffY)
                    uMaxDiffY = uDiffY;
            }
        }
        u16fast uTotalSpan = uMaxDiffX + uMaxDiffY + 1;
        return float(uTotalSpan);
    } else {
        // if input has only one sheet, we're considered '2D'
        u16fast uMaxDiffX = 0u;
        u16fast uMaxDiffY = 0u;
        u16fast uCount = segment._uCount;
        const uint16* pPreSyn = segment._tPreSynIndex;
        const VANILLA_SP_SYN_PERM_TYPE* pPerm = segment._tPermValue;
        for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
            if (*pPerm >= VANILLA_SP_SYN_CONNECTED_PERM) {
                u16fast uPreSynCellIndex = *pPreSyn;
                u16fast uPreSynCellY = uPreSynCellIndex & VANILLA_HTM_SHEET_YMASK;
                u16fast uPreSynCellX = uPreSynCellIndex >> VANILLA_HTM_SHEET_SHIFT_DIVY;
                u16fast uDiffX = wrappedDistanceBetween(uPreSynCellX, uX, VANILLA_HTM_SHEET_XMASK, VANILLA_HTM_SHEET_SHIFT_DIVX);
                u16fast uDiffY = wrappedDistanceBetween(uPreSynCellY, uY, VANILLA_HTM_SHEET_YMASK, VANILLA_HTM_SHEET_SHIFT_DIVY);
                if (uDiffX > uMaxDiffX)
                    uMaxDiffX = uDiffX;
                if (uDiffY > uMaxDiffY)
                    uMaxDiffY = uDiffY;
            }
        }
        u16fast uTotalSpan = uMaxDiffX + uMaxDiffY + 1;
        return float(uTotalSpan);
    }
}

#ifdef VANILLA_SP_USE_BOOSTING
// - - - - - - - - - - - - - - - - - - - -
// Computes the boost factor to apply to a particular column, given a target and a current active ratio
// - - - - - - - - - - - - - - - - - - - -
static uint16 _getBoostFactorUint16(float fTargetActiveRatio, float fCurrentActivRatio)
{
    // Boost factor is here a 16b fixed point, with 8b after dot (=> 256 represents 1.0)
    float fArg = (fTargetActiveRatio - fCurrentActivRatio) * VANILLA_SP_USE_BOOSTING;
    // see rationale for the linear boost function at https://discourse.numenta.org/t/mapping-the-hyper-parameter-space-of-classifcation-using-sp/6815/5
    float fBoostFactor = 1.0f + fArg;
    return uint16(std::round(fBoostFactor * 256.0f));
}
#endif

// - - - - - - - - - - - - - - - - - - - -
// Fills the table of 'P'otential synapses (and their 'P'ermanence) for a given segment,
//   while having a list of candidates at hand.
// - - - - - - - - - - - - - - - - - - - -
static void _candidatesToPandP(uint16* pCandidates, u32fast uTotalCount, u16fast uConnectedCount,
    Rand* pSynRand, VanillaSP::Segment& segment)
{
    u32fast uRemaining = uTotalCount;
    segment._uCount = 0u;
    uint16* pPreSyn = segment._tPreSynIndex;
    VANILLA_SP_SYN_PERM_TYPE* pPerm = segment._tPermValue;
    // Continue drawing synapses from the candidates, until 'uConnectedCount' of them have been chosen.
    for(u32fast uAffected = 0u; uAffected < uConnectedCount; uAffected++, pPreSyn++, pPerm++) {
        // draw one candidate from the pool at random
        u32fast uNextInRemaining = pSynRand->drawNextFromZeroToExcl(uint32(uRemaining));
        u32fast uNextIndex = pCandidates[uNextInRemaining];
        // ... removing it by simply copying previous 'last' to its now freed position
        uRemaining--;
        pCandidates[uNextInRemaining] = pCandidates[uRemaining];
        // ... now chose whether it should be an initially connected or unconnected synapse (a fixed 50% chance of each)
        uint32 uBinaryConnectedDraw = pSynRand->getNext() & 1u;
        *pPreSyn = uNextIndex;
        // ... and lerping randomly in each case: between [0..connection threshold] (for unconnected)
        //     or [connection threshold.. 1.0] (for connected)
#if   (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32)
        float fLerpDraw = pSynRand->getNextAsFloat01();
        float fBinaryConnectedDraw = float(uBinaryConnectedDraw);
        float fLerpedValIfConnected = (1.0f - VANILLA_SP_SYN_CONNECTED_PERM) * fLerpDraw;
        float fPerm = fBinaryConnectedDraw * (VANILLA_SP_SYN_CONNECTED_PERM + fLerpedValIfConnected);
        fPerm += (1.0f - fBinaryConnectedDraw) * (fLerpDraw * VANILLA_SP_SYN_CONNECTED_PERM);
        *pPerm = fPerm;
#elif (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16)
        int32 iLerpDraw = int32(uint8(pSynRand->getNext()));
        int32 iFixPt16bLerpedValIfConnected = ((65535 - int32(VANILLA_SP_SYN_CONNECTED_PERM)) * iLerpDraw) >> 8;
        int32 iPerm = int32(uBinaryConnectedDraw) * (int32(VANILLA_SP_SYN_CONNECTED_PERM) + iFixPt16bLerpedValIfConnected);
        iPerm += int32(1u-uBinaryConnectedDraw) * ((iLerpDraw * int32(VANILLA_SP_SYN_CONNECTED_PERM)) >> 8);
        *pPerm = uint16(iPerm);
#elif (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8)
        int32 iLerpDraw = int32(uint8(pSynRand->getNext()));
        int32 iFixPt8bLerpedValIfConnected = ((255 - int32(VANILLA_SP_SYN_CONNECTED_PERM)) * iLerpDraw) >> 8;
        int32 iPerm = int32(uBinaryConnectedDraw) * (int32(VANILLA_SP_SYN_CONNECTED_PERM) + iFixPt8bLerpedValIfConnected);
        iPerm += int32(1u-uBinaryConnectedDraw) * ((iLerpDraw * int32(VANILLA_SP_SYN_CONNECTED_PERM)) >> 8);
        *pPerm = uint8(iPerm);
#else
#  error "candidatesToPandP not yet implemented for this value of VANILLA_SP_SYNAPSE_KIND"
#endif
    }
    // and now that we've indeed chosen 'uConnectedCount' synapses to be added, don't forget to update that little guy, of course
    segment._uCount = uint16(uConnectedCount);
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and unsigned decrease to apply,
//   when the potential connection area in fact covers the whole sheet
// - - - - - - - - - - - - - - - - - - - -
static void _initMapPotentialsGlobal(VanillaSP::Segment& segment, u16fast uX, u16fast uY, Rand* pSynRand,
    u32fast uTotalCount, u16fast uConnectedCount, uint16* pTmpBuffer)
{
    uint16* pCurrent = pTmpBuffer;
    for (u32fast uCandidateIndex = 0u; uCandidateIndex < uTotalCount; uCandidateIndex++, pCurrent++) {
        *pCurrent = uint16(uCandidateIndex);
    }
    _candidatesToPandP(pTmpBuffer, uTotalCount, uConnectedCount, pSynRand, segment);
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and unsigned decrease to apply,
//   when the potential connection area is beyond sheet height
// - - - - - - - - - - - - - - - - - - - -
static void _initMapPotentialsLocalAlongX(VanillaSP::Segment& segment, u16fast uX, u16fast uY, Rand* pSynRand,
    u16fast uSizeX, u16fast uSizeZ, u16fast uPotentialRadius, u32fast uTotalCount, u16fast uConnectedCount, uint16* pTmpBuffer)
{
    uint16* pCurrent = pTmpBuffer;
    u16fast uStartZIndex = 0u;
    for (u16fast uCandidateZ = 0u; uCandidateZ < uSizeZ; uCandidateZ++, uStartZIndex += VANILLA_HTM_SHEET_2DSIZE) {
        for (u16fast uCandidateRelX = 0u; uCandidateRelX < uSizeX; uCandidateRelX++) {
            u16fast uCandidateX = u16fast(uX - uPotentialRadius + uCandidateRelX) & VANILLA_HTM_SHEET_XMASK;
            u16fast uIndex = uStartZIndex + (uCandidateX << VANILLA_HTM_SHEET_SHIFT_DIVY);
            for (u16fast uCandidateY = 0u; uCandidateY < VANILLA_HTM_SHEET_2DSIZE; uCandidateY++, uIndex++, pCurrent++) {
                *pCurrent = uIndex;
            }
        }
    }
    _candidatesToPandP(pTmpBuffer, uTotalCount, uConnectedCount, pSynRand, segment);
}

// - - - - - - - - - - - - - - - - - - - -
// Returns an updated synaptic permanence value, knowing previous permanence and unsigned decrease to apply,
//   when the potential connection area is reasonnable (such as from default radius 12)
// - - - - - - - - - - - - - - - - - - - -
static void _initMapPotentialsFullyLocal(VanillaSP::Segment& segment, u16fast uX, u16fast uY, Rand* pSynRand,
    u16fast uSizeXY, u16fast uSizeZ, u16fast uRadius, u32fast uTotalCount, u16fast uConnectedCount, uint16* pTmpBuffer)
{
    uint16* pCurrent = pTmpBuffer;
    u16fast uStartZIndex = 0u;
    for (u16fast uCandidateZ = 0u; uCandidateZ < uSizeZ; uCandidateZ++, uStartZIndex += VANILLA_HTM_SHEET_2DSIZE) {
        for (u16fast uCandidateRelX = 0u; uCandidateRelX < uSizeXY; uCandidateRelX++) {
            u16fast uCandidateX = u16fast(uX - uRadius + uCandidateRelX) & VANILLA_HTM_SHEET_XMASK;
            u16fast uStartXIndex = uStartZIndex + (uCandidateX << VANILLA_HTM_SHEET_SHIFT_DIVY);
            for (u16fast uCandidateRelY = 0u; uCandidateRelY < uSizeXY; uCandidateRelY++, pCurrent++) {
                u16fast uCandidateY = u16fast(uY - uRadius + uCandidateRelY) & VANILLA_HTM_SHEET_YMASK;
                *pCurrent = uint16(uStartXIndex + uCandidateY);
            }
        }
    }
    _candidatesToPandP(pTmpBuffer, uTotalCount, uConnectedCount, pSynRand, segment);
}

// - - - - - - - - - - - - - - - - - - - -
// Method used in case the VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI option is chosen:
//   Required to compute the initial (after first init of potentials and permanences) connectivity field for a segment.
//   After init, we won't use this same method, as we never brute-force or way through it: we'll rather update whenever a synapse
//     changes from connected to unconnected status (or the other way around)
// - - - - - - - - - - - - - - - - - - - -
static void _initConnectivityField(const VanillaSP::Segment& segment, uint64* pConnectivityField)
{
    memset((void*)pConnectivityField, 0, VANILLA_HTM_SHEET_2DSIZE >> 3u);
    u16fast uCount = segment._uCount;
    const uint16* pPreSyn = segment._tPreSynIndex;
    const VANILLA_SP_SYN_PERM_TYPE* pPerm = segment._tPermValue;
    for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
        if (*pPerm >= VANILLA_SP_SYN_CONNECTED_PERM) {
            u16fast uIndex = *pPreSyn;
            u16fast uQword = uIndex >> 6u;
            u16fast uBit = uIndex & 0x003Fu;
            pConnectivityField[uQword] |= (1uLL << uBit);
        }
    }
}

// central factor for '_computeGaussian' implementation
static const uint32 k_uFactorForCenter = 327u;

// 15 factors along one orientation for '_getGaussianSum' implementation
static const uint32 k_gaussianFactors[15u] = {
    320u, 302u, 273u, 237u, 199u,
    159u, 123u,  91u,  65u,  45u,
    29u,  19u,  11u,   7u,   4u,
};

// - - - - - - - - - - - - - - - - - - - -
// returns the sum of 15 samples along one axis and one direction, weighted by factors following a gaussian curve
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bIsY, bool bBackwards, bool bCareForWrap>
static uint32 _getGaussianSum(const ActivationLevelType* pActivationLevelsPerCol, u16fast uX, u16fast uY)
{
    uint32 uSum = 0u;
    for (u16fast uOffset = 1u; uOffset <= 15u; uOffset++) {
        u16fast uSampleX = i16fast(uX);
        u16fast uSampleY = i16fast(uY);
        if (bIsY) {
            if (bCareForWrap) {
                uSampleY += bBackwards ? (VANILLA_HTM_SHEET_HEIGHT-uOffset) : uOffset;
                uSampleY &= VANILLA_HTM_SHEET_YMASK;
            } else {
                if (bBackwards)
                    uSampleY -= uOffset;
                else
                    uSampleY += uOffset;
            }
        } else {
            if (bCareForWrap) {
                uSampleX += bBackwards ? (VANILLA_HTM_SHEET_WIDTH-uOffset) : uOffset;
                uSampleX &= VANILLA_HTM_SHEET_XMASK;
            } else {
                if (bBackwards)
                    uSampleX -= uOffset;
                else
                    uSampleX += uOffset;
            }
        }
        u16fast uSampleIndex = (uSampleX * VANILLA_HTM_SHEET_HEIGHT) + uSampleY;
        uSum += uint32(pActivationLevelsPerCol[uSampleIndex]) * k_gaussianFactors[uOffset-1u];
    }
    return uSum;
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// computes a gaussian filter over a 31x31 kernel, using the well known two-passes optimization (one for each dimension)
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bOutputMinActivation>
static void _computeGaussian(const ActivationLevelType* pActivationLevelsPerCol, uint32* pOutY, uint32* pOutFinal,
    uint32* pOutputMinActivation)
{
    const ActivationLevelType* pCurrentActivation = pActivationLevelsPerCol;
    uint32* pCurrentOut = pOutY;
    // sum of gaussian factors is 4095 => shift by 12 nominally, or shift by 4 for first round of y
    //   for when ActivationLevelType is 16b (=> "raw") so as to simulate boosted-by-1.0 (towards fixed pt 8b after point)
    static const u16fast uShiftFirst = (sizeof(ActivationLevelType) == 2u) ? 4u : 12u;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uY = 0u;
        for (; uY < VANILLA_HTM_SHEET_HALFHEIGHT; uY++, pCurrentActivation++, pCurrentOut++) {
            uint32 uCurrentSum = uint32(*pCurrentActivation) * k_uFactorForCenter;
            uCurrentSum += _getGaussianSum<ActivationLevelType, true, true, true>(pActivationLevelsPerCol, uX, uY);
            uCurrentSum += _getGaussianSum<ActivationLevelType, true, false, false>(pActivationLevelsPerCol, uX, uY);
            uint32 uValue = uCurrentSum >> uShiftFirst;
            *pCurrentOut = uValue;
        }
        for (; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentActivation++, pCurrentOut++) {
            uint32 uCurrentSum = uint32(*pCurrentActivation) * k_uFactorForCenter;
            uCurrentSum += _getGaussianSum<ActivationLevelType, true, true, false>(pActivationLevelsPerCol, uX, uY);
            uCurrentSum += _getGaussianSum<ActivationLevelType, true, false, true>(pActivationLevelsPerCol, uX, uY);
            uint32 uValue = uCurrentSum >> uShiftFirst;
            *pCurrentOut = uValue;
        }
    }
    const uint32* pCurrentY = pOutY;
    uint32* pCurrentMin = pOutputMinActivation;
    pCurrentOut = pOutFinal;
    u16fast uX = 0u;
    for (; uX < VANILLA_HTM_SHEET_HALFWIDTH; uX++) {
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentY++, pCurrentOut++) {
            uint32 uCurrentSum = (*pCurrentY) * k_uFactorForCenter;
            uCurrentSum += _getGaussianSum<uint32, false, true, true>(pOutY, uX, uY);
            uCurrentSum += _getGaussianSum<uint32, false, false, false>(pOutY, uX, uY);
            uint32 uValue = uCurrentSum >> 12u;
            *pCurrentOut = uValue;
            if (bOutputMinActivation) {
                *pCurrentMin += uValue;
                pCurrentMin++;
            }
        }
    }
    for (; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentY++, pCurrentOut++) {
            uint32 uCurrentSum = (*pCurrentY) * k_uFactorForCenter;
            uCurrentSum += _getGaussianSum<uint32, false, true, false>(pOutY, uX, uY);
            uCurrentSum += _getGaussianSum<uint32, false, false, true>(pOutY, uX, uY);
            uint32 uValue = uCurrentSum >> 12u;
            *pCurrentOut = uValue;
            if (bOutputMinActivation) {
                *pCurrentMin += uValue;
                pCurrentMin++;
            }
        }
    }
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// reduces a table of activation levels, given a table of reduction values. clamp results to zero before assigning to 'pResult'
// NB : pResult MAY alias pActivationLevelsPerCol
// returns number of remaining non-zeros (computed with unbranching methods)
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType>
static u16fast _reduceByAmount(const ActivationLevelType* pActivationLevelsPerCol, const uint32* pReduction, uint32* pResult) {
    u16fast uNonZeroCount = 0u;
    const ActivationLevelType* pCurrentActivation = pActivationLevelsPerCol;
    const uint32* pCurrentReduction = pReduction;
    uint32* pCurrentResult = pResult ;
    for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE;
            uIndex++, pCurrentActivation++, pCurrentReduction++, pCurrentResult++) {
        int32 iReduced = (sizeof(ActivationLevelType) == 2u) ? 
            ((int32(*pCurrentActivation) << 8) - int32(*pCurrentReduction)) :   // if raw, shift to 8b after point
            (int32(*pCurrentActivation) - int32(*pCurrentReduction));           // otherwise already both 8b after point
        int32 iMaskIfNonNeg = ~(iReduced >> 31);
        *pCurrentResult = iMaskIfNonNeg & iReduced;
        uNonZeroCount += u16fast(iMaskIfNonNeg & 1u);
    }
    return uNonZeroCount;
}
; // template termination

static u16fast _reduceByAmountScaled(const uint32* pStartLevelsPerCol, const uint32* pReduction,
    uint32 uScale8bAfterPoint, uint32* pResult)
{
    u16fast uNonZeroCount = 0u;
    const uint32* pCurrentActivation = pStartLevelsPerCol;
    const uint32* pCurrentReduction = pReduction;
    uint32* pCurrentResult = pResult ;
    for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE;
        uIndex++, pCurrentActivation++, pCurrentReduction++, pCurrentResult++) {
        int32 iReduced = int32(*pCurrentActivation) - ((int32(uScale8bAfterPoint) * int32(*pCurrentReduction)) >> 8);
        int32 iMaskIfNonNeg = ~(iReduced >> 31);
        *pCurrentResult = iMaskIfNonNeg & iReduced;
        uNonZeroCount += u16fast(iMaskIfNonNeg & 1u);
    }
    return uNonZeroCount;
}

// - - - - - - - - - - - - - - - - - - - -
// VanillaSP ctor
// - - - - - - - - - - - - - - - - - - - -
VanillaSP::VanillaSP(uint8 uNumberOfInputSheets, uint8 uPotentialConnectivityRadius, float fPotentialConnectivityRatio,
                     float fActivationDensityRatio, float fOverThresholdTargetVsMaxRatio, uint64 uColumnUsageIntegrationWindow,
                     uint64 uSeed)
{
    static const size_t uQwordsPerBinarySheet = VANILLA_HTM_SHEET_2DSIZE >> 6u;  // 64b per Qword
    _pTmpBinaryInputBuffer = new uint64[size_t(uNumberOfInputSheets) * uQwordsPerBinarySheet];
    _pTmpBinaryOutputBuffer = new uint64[uQwordsPerBinarySheet];
    _pTmpBinaryOverThresholdActivations = new uint64[uQwordsPerBinarySheet];

    _uInputSheetsCount = uNumberOfInputSheets;
    _uPotentialConnectivityRadius = uPotentialConnectivityRadius;
    u8fast uPotentialConnectivitySideSize = 1u + 2u * uPotentialConnectivityRadius;
    _fPotentialConnectivityRatio = fPotentialConnectivityRatio;
    _fActivationDensityRatio = fActivationDensityRatio;
    _fOverThresholdTargetVsMaxRatio = fOverThresholdTargetVsMaxRatio;
    _uColumnUsageIntegrationWindow = uColumnUsageIntegrationWindow;

    _uEpoch = 0u;
    _uEpochLearning = 0u;
    _pAverageActiveRatioPerColumn = new float[VANILLA_HTM_SHEET_2DSIZE];
    _pAverageOverThresholdRatioPerColumn = new float[VANILLA_HTM_SHEET_2DSIZE];
    _pOverThresholdRatioTargetPerColumn = new float[VANILLA_HTM_SHEET_2DSIZE];
    for (size_t uCol = 0; uCol < size_t(VANILLA_HTM_SHEET_2DSIZE); uCol++) {
        _pAverageActiveRatioPerColumn[uCol] = fActivationDensityRatio;
        _pAverageOverThresholdRatioPerColumn[uCol] = VANILLA_SP_OVERTHRESHOLD_INIT;
        _pOverThresholdRatioTargetPerColumn[uCol] = VANILLA_SP_OVERTHRESHOLD_INIT * VANILLA_SP_DEFAULT_TARGET_VS_MAX_RATIO;
    }
    _pTmpRawActivationLevelsPerCol = new uint16[VANILLA_HTM_SHEET_2DSIZE];
#ifdef VANILLA_SP_USE_BOOSTING
    _pTmpBoostedActivationLevelsPerCol = new uint32[VANILLA_HTM_SHEET_2DSIZE];
    _pBoostingPerCol = new uint16[VANILLA_HTM_SHEET_2DSIZE];
    for (uint16 *pCurrentBoosting = _pBoostingPerCol, *pEnd = _pBoostingPerCol + VANILLA_HTM_SHEET_2DSIZE;
            pCurrentBoosting < pEnd; pCurrentBoosting++) {
        *pCurrentBoosting = 256u;
    }
#endif

    _pSegments = new Segment[VANILLA_HTM_SHEET_2DSIZE];
    Rand synRand;
    if (uSeed) // a value of 0 for uSeed (the default) will let the 'Rand' implementation choose its default seed of choice
        synRand.seed(uint32(uSeed));
    Segment* pCurrentSeg = _pSegments;
    if (uPotentialConnectivitySideSize < VANILLA_SP_MIN_AREA_SIDE_SIZE ||
        uPotentialConnectivitySideSize >= VANILLA_HTM_SHEET_WIDTH) {
        u32fast uTotalCount = VANILLA_HTM_SHEET_2DSIZE * size_t(uNumberOfInputSheets);
        u32fast uConnectedCount = u32fast(std::round(float(uTotalCount) * fPotentialConnectivityRatio));
        u32fast uMaxCount = std::min(uTotalCount-u32fast(1u), u32fast(VANILLA_SP_MAX_SYNAPSES_PER_SEG));
        uConnectedCount = std::min(uMaxCount, uConnectedCount);
        uConnectedCount = std::max(u32fast(1u), uConnectedCount);
        uint16* pTmpBuffer = new uint16[uTotalCount];
        for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
            for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentSeg++) {
                _initMapPotentialsGlobal(*pCurrentSeg, uX, uY, &synRand, uTotalCount, u16fast(uConnectedCount), pTmpBuffer);
            }
        }
        delete[] pTmpBuffer;
    } else if (uPotentialConnectivitySideSize >= VANILLA_HTM_SHEET_HEIGHT) {
        u32fast uTotalCount = u32fast(uPotentialConnectivitySideSize) * VANILLA_HTM_SHEET_HEIGHT * size_t(uNumberOfInputSheets);
        u32fast uConnectedCount = u32fast(std::round(float(uTotalCount) * fPotentialConnectivityRatio));
        u32fast uMaxCount = std::min(uTotalCount-u32fast(1u), u32fast(VANILLA_SP_MAX_SYNAPSES_PER_SEG));
        uConnectedCount = std::min(uMaxCount, uConnectedCount);
        uConnectedCount = std::max(u32fast(1u), uConnectedCount);
        uint16* pTmpBuffer = new uint16[uTotalCount];
        for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
            for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentSeg++) {
                _initMapPotentialsLocalAlongX(*pCurrentSeg, uX, uY, &synRand, uPotentialConnectivitySideSize,
                    uNumberOfInputSheets, uPotentialConnectivityRadius, uTotalCount, u16fast(uConnectedCount), pTmpBuffer);
            }
        }
        delete[] pTmpBuffer;
    } else {
        u32fast uSq = u32fast(uPotentialConnectivitySideSize) * u32fast(uPotentialConnectivitySideSize);
        u32fast uTotalCount = uSq * u32fast(uNumberOfInputSheets);
        u32fast uConnectedCount = u32fast(std::round(float(uTotalCount) * fPotentialConnectivityRatio));
        u32fast uMaxCount = std::min(uTotalCount-u32fast(1u), u32fast(VANILLA_SP_MAX_SYNAPSES_PER_SEG));
        uConnectedCount = std::min(uMaxCount, uConnectedCount);
        uConnectedCount = std::max(u32fast(1u), uConnectedCount);
        uint16* pTmpBuffer = new uint16[uTotalCount];
        for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
            for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentSeg++) {
                _initMapPotentialsFullyLocal(*pCurrentSeg, uX, uY, &synRand, uPotentialConnectivitySideSize,
                    uNumberOfInputSheets, uPotentialConnectivityRadius, uTotalCount, u16fast(uConnectedCount), pTmpBuffer);
            }
        }
        delete[] pTmpBuffer;
    }

#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
    _uConnectivityFieldsQwordSizePerColumn = size_t(uNumberOfInputSheets) * uQwordsPerBinarySheet;
    _pConnectivityFields = new uint64[VANILLA_HTM_SHEET_2DSIZE * _uConnectivityFieldsQwordSizePerColumn];
    uint64* pCurrentField = _pConnectivityFields;
    pCurrentSeg = _pSegments;
    for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE;
            uIndex++, pCurrentSeg++, pCurrentField += _uConnectivityFieldsQwordSizePerColumn) {
        _initConnectivityField(*pCurrentSeg, pCurrentField);
    }
#endif

    _uInhibitionRadius = VANILLA_HTM_SHEET_HEIGHT;
    _uInhibitionSideSize = 1u + 2u * _uInhibitionRadius;
    u16fast uMaxK_now = u16fast(std::round(float(VANILLA_HTM_SHEET_2DSIZE) * fActivationDensityRatio));
    uMaxK_now = std::min(uMaxK_now, u16fast(VANILLA_SP_MAX_WINNERS));
    uMaxK_now = std::max(uMaxK_now, u16fast(1u));
    _uCurrentWinnerK = uMaxK_now;
    _pTmpTableBest = new uint32[uMaxK_now + 1u];
#ifdef VANILLA_SP_USE_LOCAL_INHIB
    _onUpdateDynamicInhibitionRange();
#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
#    if (VANILLA_SP_NEIGHBORHOOD_OPTIM == VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSFILTER)
        _pTmpGaussY = new uint32[VANILLA_HTM_SHEET_2DSIZE];
        _pTmpGaussX = new uint32[VANILLA_HTM_SHEET_2DSIZE];
        _pReducedActivations = new uint32[VANILLA_HTM_SHEET_2DSIZE];
#    endif // VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSTEST or VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_ENFSPACING
#  endif // valueof VANILLA_SP_USE_LOCAL_INHIB
#endif // VANILLA_SP_USE_LOCAL_INHIB
}

// - - - - - - - - - - - - - - - - - - - -
// VanillaSP dtor
// - - - - - - - - - - - - - - - - - - - -
VanillaSP::~VanillaSP()
{
    delete[] _pTmpBinaryInputBuffer;
    delete[] _pTmpBinaryOutputBuffer;
    delete[] _pTmpBinaryOverThresholdActivations;

    delete[] _pAverageActiveRatioPerColumn;
    delete[] _pAverageOverThresholdRatioPerColumn;
    delete[] _pOverThresholdRatioTargetPerColumn;

    delete[] _pTmpRawActivationLevelsPerCol;
#ifdef VANILLA_SP_USE_BOOSTING
    delete[] _pTmpBoostedActivationLevelsPerCol;
    delete[] _pBoostingPerCol;
#endif

    delete[] _pTmpTableBest;

    delete[] _pSegments;
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
    delete[] _pConnectivityFields;
#endif

#ifdef VANILLA_SP_USE_LOCAL_INHIB
#if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
#    if (VANILLA_SP_NEIGHBORHOOD_OPTIM == VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSFILTER)
        delete[] _pTmpGaussY;
        delete[] _pTmpGaussX;
        delete[] _pReducedActivations;
#    endif // VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSTEST or VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_ENFSPACING
#  endif // valueof VANILLA_SP_USE_LOCAL_INHIB
#endif // VANILLA_SP_USE_LOCAL_INHIB
}

// - - - - - - - - - - - - - - - - - - - -
// VanillaSP main '_compute' method
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_compute(const uint64* pInputBinaryBitmap, std::vector<uint16>& vecOutputIndices, bool bLearning,
    uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations)
{
    vecOutputIndices.clear();
    _uEpoch++;
    _computeUnrestrictedActivationLevels(pInputBinaryBitmap, _pTmpRawActivationLevelsPerCol);
#if defined(VANILLA_SP_USE_BOOSTING)
    _computeActiveColumnsAndLearnWhenBoosted(pInputBinaryBitmap, vecOutputIndices, bLearning, pOutputBinaryBitmap, pOutputMinActivations);
#else
    _computeActiveColumnsAndLearnWhenNoBoosting(pInputBinaryBitmap, vecOutputIndices, bLearning, pOutputBinaryBitmap, pOutputMinActivations);
#endif
    if (bLearning) {
        _uEpochLearning++;
        if (0uLL == (_uEpochLearning & 0x003FuLL)) { // complex updates are called once every 64 rounds
#  if defined(VANILLA_SP_USE_LOCAL_INHIB) && !defined(VANILLA_SP_FORCE_NONLOCAL_STATS)
            _onUpdateDynamicInhibitionRange();
#    if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
            if (_uInhibitionSideSize < VANILLA_SP_MIN_AREA_SIDE_SIZE || _uInhibitionSideSize >= VANILLA_HTM_SHEET_WIDTH) {
                _onUpdateOverThresholdRatioTargetWithGlobalInhib();
            } else if (_uInhibitionSideSize >= VANILLA_HTM_SHEET_HEIGHT) {
                _onUpdateOverThresholdRatioTargetWithLocalInhibAlongX();
            } else {
                _onUpdateOverThresholdRatioTargetWithFullLocalInhib();
            }
#    else // hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
            _onUpdateOverThresholdRatioTargetWithBucketInhib();
#    endif // value of VANILLA_SP_USE_LOCAL_INHIB 
#  else  // !VANILLA_SP_USE_LOCAL_INHIB || VANILLA_SP_FORCE_NONLOCAL_STATS
            _onUpdateOverThresholdRatioTargetWithGlobalInhib();
#  endif // VANILLA_SP_USE_LOCAL_INHIB
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_computeUnrestrictedActivationLevels(const uint64* pInputBinaryBitmap,
    uint16* pOutputActivationLevelsPerCol) const
{
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
    const uint64* pCurrentConnectivityFieldQword = _pConnectivityFields;
    const uint64 uIterCount = _uConnectivityFieldsQwordSizePerColumn;
    for (uint16 *pCurrentColOutput = pOutputActivationLevelsPerCol,
            *pEndOutput = pOutputActivationLevelsPerCol + VANILLA_HTM_SHEET_2DSIZE;
            pCurrentColOutput < pEndOutput; pCurrentColOutput++) {
        uint64 uLevelOnThisColumn = 0uLL;
        for (const uint64 *pCurrentInputQword = pInputBinaryBitmap, *pEndInput = pInputBinaryBitmap+uIterCount;
                pCurrentInputQword < pEndInput; pCurrentInputQword++, pCurrentConnectivityFieldQword++) {
            uint64 uOverlapOnThisQword = (*pCurrentConnectivityFieldQword) & (*pCurrentInputQword);
            uLevelOnThisColumn += countSetBits64(uOverlapOnThisQword);
        }
        *pCurrentColOutput = uint16(uLevelOnThisColumn);
    }
#else
    const Segment *pCurrentSeg = _pSegments;
    for (uint16 *pCurrentColOutput = pOutputActivationLevelsPerCol,
            *pEndOutput = pOutputActivationLevelsPerCol + VANILLA_HTM_SHEET_2DSIZE;
            pCurrentColOutput < pEndOutput; pCurrentColOutput++, pCurrentSeg++) {
        uint64 uLevelOnThisColumn = 0uLL;
        u16fast uCount = pCurrentSeg->_uCount;
        const uint16* pPreSyn = pCurrentSeg->_tPreSynIndex;
        const VANILLA_SP_SYN_PERM_TYPE* pPerm = pCurrentSeg->_tPermValue;
        for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
            if (*pPerm >= VANILLA_SP_SYN_CONNECTED_PERM) {
                u16fast uIndex = *pPreSyn;
                u16fast uQword = uIndex >> 6u;
                u16fast uBit = uIndex & 0x003Fu;
                uLevelOnThisColumn += (pInputBinaryBitmap[uQword] >> uBit) & 1uLL;
            }
        }
        *pCurrentColOutput = uint16(uLevelOnThisColumn);
    }
#endif
}


#ifdef VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_computeBoostedActivationLevels(const uint16* pActivationLevelsPerCol,
        uint32* pOutputBoostedActivationLevelsPerCol) const
{
    uint32* pCurrentColOutput = pOutputBoostedActivationLevelsPerCol;
    const uint16* pCurrentColBoosting = _pBoostingPerCol;
    for (const uint16 *pCurrentColInput = pActivationLevelsPerCol, *pEndInput = pActivationLevelsPerCol+VANILLA_HTM_SHEET_2DSIZE;
        pCurrentColInput < pEndInput; pCurrentColInput++, pCurrentColBoosting++, pCurrentColOutput++) {
        *pCurrentColOutput = uint32(*pCurrentColInput) * uint32(*pCurrentColBoosting);
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onEvaluateBoostingFromColumnUsageWithGlobalInhib()
{
    float fLocalActivityAverage = _getSumFromRange<false, false>(0u, VANILLA_HTM_SHEET_WIDTH, 0u, VANILLA_HTM_SHEET_HEIGHT,
        _pAverageActiveRatioPerColumn) / float(VANILLA_HTM_SHEET_2DSIZE);
    uint16* pCurrentBoosting = _pBoostingPerCol;
    const float* pCurrentActivRatio = _pAverageActiveRatioPerColumn;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentBoosting++, pCurrentActivRatio++) {
            *pCurrentBoosting = _getBoostFactorUint16(fLocalActivityAverage, *pCurrentActivRatio);
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_computeActiveColumnsAndLearnWhenBoosted(const uint64* pInputBinaryBitmap,
    std::vector<uint16>& vecOutputIndices, bool bLearning, uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations)
{
    _computeBoostedActivationLevels(_pTmpRawActivationLevelsPerCol, _pTmpBoostedActivationLevelsPerCol);
    _getActiveColumnsFromActivationLevels(_pTmpBoostedActivationLevelsPerCol, vecOutputIndices, pOutputMinActivations);
    if (bLearning || pOutputBinaryBitmap) {
        if (!pOutputBinaryBitmap)
            pOutputBinaryBitmap = _pTmpBinaryOutputBuffer;
        SDRTools::toBinaryBitmap64(vecOutputIndices, pOutputBinaryBitmap, VANILLA_HTM_SHEET_BYTES_BINARY);
        if (bLearning) {
            _updateSynapsesOnActiveColumnsTowardsCurrentInput(pInputBinaryBitmap, vecOutputIndices);
            _onEvaluateColumnUsage(_pTmpRawActivationLevelsPerCol, pOutputBinaryBitmap);
            _onIncreasePermanencesForUnderUsedColums();
#  if defined(VANILLA_SP_USE_LOCAL_INHIB) && !defined(VANILLA_SP_FORCE_NONLOCAL_STATS)
#    if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
            if (_uInhibitionSideSize < VANILLA_SP_MIN_AREA_SIDE_SIZE || _uInhibitionSideSize >= VANILLA_HTM_SHEET_WIDTH) {
                _onEvaluateBoostingFromColumnUsageWithGlobalInhib();
            } else if (_uInhibitionSideSize >= VANILLA_HTM_SHEET_HEIGHT) {
                _onEvaluateBoostingFromColumnUsageWithLocalInhibAlongX();
            } else {
                _onEvaluateBoostingFromColumnUsageWithFullLocalInhib();
            }
#    else // hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
            _onEvaluateBoostingFromColumnUsageWithBucketInhib();
#    endif // value of VANILLA_SP_USE_LOCAL_INHIB 
#  else  // !VANILLA_SP_USE_LOCAL_INHIB || VANILLA_SP_FORCE_NONLOCAL_STATS
            _onEvaluateBoostingFromColumnUsageWithGlobalInhib();
#  endif // VANILLA_SP_USE_LOCAL_INHIB
        }
    }
}

#else // !VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_computeActiveColumnsAndLearnWhenNoBoosting(const uint64* pInputBinaryBitmap,
    std::vector<uint16>& vecOutputIndices, bool bLearning, uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations)
{
    _getActiveColumnsFromActivationLevels(_pTmpRawActivationLevelsPerCol, vecOutputIndices, pOutputMinActivations);
    if (bLearning || pOutputBinaryBitmap) {
        if (!pOutputBinaryBitmap)
            pOutputBinaryBitmap = _pTmpBinaryOutputBuffer;
        SDRTools::toBinaryBitmap64(vecOutputIndices, pOutputBinaryBitmap, VANILLA_HTM_SHEET_BYTES_BINARY);
        if (bLearning) {
            _updateSynapsesOnActiveColumnsTowardsCurrentInput(pInputBinaryBitmap, vecOutputIndices);
            _onEvaluateColumnUsage(_pTmpRawActivationLevelsPerCol, pOutputBinaryBitmap);
            _onIncreasePermanencesForUnderUsedColums();
        }
    }
}

#endif // VANILLA_SP_USE_BOOSTING

#ifdef VANILLA_SP_USE_LOCAL_INHIB

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onUpdateDynamicInhibitionRange() {
#if (VANILLA_SP_UPDATERAD_KIND == VANILLA_SP_UPDATERAD_KIND_CONST_NOUPDATE)
    //_uInhibitionRadius = std::max(uint8(_uPotentialConnectivityRadius >> 1u), uint8(2u));   // fixing inhib radius to half potential radius
    // temporary hack to get same result as vanilla in one of the test cases
    _uInhibitionRadius = _uPotentialConnectivityRadius - 3u;
#else
    // somewhat contrived... to get same behavior as vanilla SP
    float fAvgConnectedSpan = 0.0f;
    const Segment* pCurrentSeg = _pSegments;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentSeg++) {
            fAvgConnectedSpan += _computeCorrectedAvgConnectedSpanFor(uX, uY, _uInputSheetsCount, *pCurrentSeg);
        }
    }
    fAvgConnectedSpan /= float(VANILLA_HTM_SHEET_2DSIZE);
    float fRadius = (fAvgConnectedSpan - 1.0f) * 0.5f;
    _uInhibitionRadius = std::max(uint8(1u), uint8(std::min(255.0f, std::round(fRadius))));
    //--------
    // temporary hack to get same result as vanilla in one of the test cases
#  if defined(VANILLA_SP_UPDATERAD_KIND)
#    if (VANILLA_SP_UPDATERAD_KIND != VANILLA_SP_UPDATERAD_KIND_CONST_CORRECTED)
       _uInhibitionRadius -= 3u;
#    endif
#  else
       _uInhibitionRadius -= 3u;
#  endif
    //--------
#endif // valueof VANILLA_SP_UPDATERAD_KIND
    _uInhibitionSideSize = 1u + 2u * _uInhibitionRadius;

    u16fast uCompetitorsCount;
#if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
    if (_uInhibitionSideSize >= VANILLA_HTM_SHEET_WIDTH) {
        uCompetitorsCount = VANILLA_HTM_SHEET_2DSIZE;
    } else if (_uInhibitionSideSize >= VANILLA_HTM_SHEET_HEIGHT) {
        uCompetitorsCount = u16fast(_uInhibitionSideSize) * u16fast(VANILLA_HTM_SHEET_HEIGHT);
    } else {
        uCompetitorsCount = u16fast(_uInhibitionSideSize) * u16fast(_uInhibitionSideSize);
    }
#  else // hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
    _uBucketSize = _uInhibitionRadius * 2u;
    if (_uBucketSize <= 6u) {
        _uBucketSize = 4u;
        _uBucketCountY = uint8(VANILLA_HTM_SHEET_HEIGHT >> 2u);
    } else if (_uBucketSize <= 12u) {
        _uBucketSize = 8u;
        _uBucketCountY = uint8(VANILLA_HTM_SHEET_HEIGHT >> 3u);
    } else if (_uBucketSize <= 24u) {
        _uBucketSize = 16u;
        _uBucketCountY = uint8(VANILLA_HTM_SHEET_HEIGHT >> 4u);
    } else {
        _uBucketSize = 32u;
        _uBucketCountY = uint8(VANILLA_HTM_SHEET_HEIGHT >> 5u);
    }
    uCompetitorsCount = u16fast(_uBucketSize) * u16fast(_uBucketSize);
#  endif // valueof VANILLA_SP_USE_LOCAL_INHIB
    u16fast uMaxK_now = u16fast(std::round(float(uCompetitorsCount) * _fActivationDensityRatio));
    uMaxK_now = std::min(uMaxK_now, u16fast(VANILLA_SP_MAX_WINNERS));
    uMaxK_now = std::max(uMaxK_now, u16fast(1u));
    if (uMaxK_now != _uCurrentWinnerK) {
        delete[] _pTmpTableBest;
        _pTmpTableBest = new uint32[uMaxK_now + 1u];
        _uCurrentWinnerK = size_t(uMaxK_now);
    }
}

#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)

#    if (VANILLA_SP_NEIGHBORHOOD_OPTIM == VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSFILTER)

template<typename ActivationLevelType, bool bOutputMinActivation>
void VanillaSP::_reduceActivationsByGaussianFilter(const ActivationLevelType* pActivationLevelsPerCol,
    uint32* pOutputMinActivation)
{
    if (bOutputMinActivation) {
        std::memset((void*)pOutputMinActivation, 0, sizeof(uint32_t)*VANILLA_HTM_SHEET_2DSIZE);
    }
    _computeGaussian<ActivationLevelType, bOutputMinActivation>(pActivationLevelsPerCol, _pTmpGaussY, _pTmpGaussX,
        pOutputMinActivation);
    u16fast uCurrentCount = _reduceByAmount<ActivationLevelType>(pActivationLevelsPerCol, _pTmpGaussX, _pReducedActivations);
    if (uCurrentCount >= 42) {
        // usually we won't have reached target sparsity 2% (41 active) in only one reduction-by-gaussian filter,
        //   so we still have VANILLA_SP_MAX_GAUSSIAN_ITER-1 to get closer to it
        for (u16fast uIterateMore = 1u; uIterateMore < VANILLA_SP_MAX_GAUSSIAN_ITER; uIterateMore++) {
            _computeGaussian<uint32, bOutputMinActivation>(_pReducedActivations, _pTmpGaussY, _pTmpGaussX,
                pOutputMinActivation);
            uCurrentCount = _reduceByAmount<uint32>(_pReducedActivations, _pTmpGaussX, _pReducedActivations);
            if (uCurrentCount < 42u) {
                break;
            }
        }
    }
#ifdef VANILLA_SP_GAUSSIAN_SCALE_TARGET
    if (uCurrentCount >= VANILLA_SP_GAUSSIAN_SCALE_TARGET) {
        // usually iterative reduction at weight 1 by gaussian filter won't be enough still,
        //   so we now try to reduce by reusing last gaussian filter at increased reduction weight
        // Note that we may not try for the '41' value, though... since we'd prefer to use a more suitable method to do
        //   the last few trimming steps (here we depend on VANILLA_SP_GAUSSIAN_SCALE_TARGET)
        uint32 tTmpReduced[VANILLA_HTM_SHEET_2DSIZE];
        std::memcpy((void*)tTmpReduced, _pReducedActivations, sizeof(uint32_t)*VANILLA_HTM_SHEET_2DSIZE);
        uint32 uScale8bAfterPoint = 256u;
        uint32 uScaleIncrease = 32u;
        do {
            uScale8bAfterPoint += uScaleIncrease;
            u16fast uCountNow = _reduceByAmountScaled(tTmpReduced, _pTmpGaussX, uScale8bAfterPoint, _pReducedActivations);
            if (uCountNow < 38u) {
                if (uScaleIncrease > 1u) {
                    uScale8bAfterPoint -= uScaleIncrease;
                    uScaleIncrease >>= 2u;
                } else {
                    uCurrentCount = uCountNow;
                }
            } else {
                uCurrentCount = uCountNow;
            }
        } while (uCurrentCount >= VANILLA_SP_GAUSSIAN_SCALE_TARGET);
        if (bOutputMinActivation) {
            const uint32* pCurrentReduction = _pTmpGaussX;
            uint32* pCurrentMin = pOutputMinActivation;
            for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE; uIndex++, pCurrentReduction++, pCurrentMin++) {
                *pCurrentMin += ((*pCurrentReduction) * uScale8bAfterPoint) >> 8u;
            }
        }
    }
#endif // VANILLA_SP_GAUSSIAN_SCALE_TARGET
}
; // template termination

#    endif // // VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSFILTER

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bOutputMinActivation>
void VanillaSP::_getActiveColumnsFromActivationLevelsWithLocalInhibAlongX(const ActivationLevelType* pActivationLevelsPerCol,
    std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations)
{
    // Stored column-major => use it to our advantage by only computing neighborhood best at columns
    u16fast uIndex = 0u;
    u16fast uXstartOffset = u16fast(_uInhibitionRadius);
    u16fast uXsize = 1u + uXstartOffset*2u;
    u16fast uTableSize = u16fast(_uCurrentWinnerK) + 1u;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uXstartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        u16fast uCountBest = _getBestFromRange<ActivationLevelType, true, false>(uStartX, uXsize, 0u, VANILLA_HTM_SHEET_HEIGHT,
            pActivationLevelsPerCol, _pTmpTableBest, uTableSize);
        if (uCountBest) {
            ActivationLevelType uBelowMin = ActivationLevelType(_pTmpTableBest[uCountBest]);
            for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, uIndex++) {
                if (pActivationLevelsPerCol[uIndex] > uBelowMin)
                    vecOutputIndices.push_back(uIndex);
                if (bOutputMinActivation) { // static test, shall be optimized out when false
                    *pOutputMinActivations = uBelowMin;
                    pOutputMinActivations++;
                }
            }
        } else {
            uIndex += VANILLA_HTM_SHEET_HEIGHT;
            if (bOutputMinActivation) { // static test, shall be optimized out when false
                for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pOutputMinActivations++)
                    *pOutputMinActivations = 0u;
            }
        }
    }
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bOutputMinActivation>
void VanillaSP::_getActiveColumnsFromActivationLevelsWithFullLocalInhib(const ActivationLevelType* pActivationLevelsPerCol,
    std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations)
{
    // Full neighborhood computation at each point
#if (VANILLA_SP_NEIGHBORHOOD_OPTIM == 0)
    u16fast uIndex = 0u;
    u16fast uStartOffset = size_t(_uInhibitionRadius);
    u16fast uSize = 1u + uStartOffset*2u;
    u16fast uTableSize = u16fast(_uCurrentWinnerK) + 1u;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uStartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, uIndex++) {
            u16fast uStartY = (uY - uStartOffset) & VANILLA_HTM_SHEET_YMASK;    // wrapping around 32 Y-positions
            u16fast uCountBest = _getBestFromRange<ActivationLevelType, true, true>(uStartX, uSize, uStartY, uSize,
                pActivationLevelsPerCol, _pTmpTableBest, uTableSize);
            if (uCountBest) {
                ActivationLevelType uBelowMin = ActivationLevelType(_pTmpTableBest[uCountBest]);
                if (pActivationLevelsPerCol[uIndex] > uBelowMin)
                    vecOutputIndices.push_back(uIndex);
                if (bOutputMinActivation) { // static test, shall be optimized out when false
                    *pOutputMinActivations = uBelowMin;
                    pOutputMinActivations++;
                }
            } else if (bOutputMinActivation) { // static test, shall be optimized out when false
                *pOutputMinActivations = 0u;
                pOutputMinActivations++;
            }
        }
    }
#elif (VANILLA_SP_NEIGHBORHOOD_OPTIM == VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_ALGORITHM)
    // TODO
#   error ("_getActiveColumnsFromActivationLevelsWithFullLocalInhib not yet implemented for AlgorithmOpti")
#elif (VANILLA_SP_NEIGHBORHOOD_OPTIM == VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSFILTER)
    _reduceActivationsByGaussianFilter<ActivationLevelType, bOutputMinActivation>(pActivationLevelsPerCol, pOutputMinActivations);
#  if defined(VANILLA_SP_ADD_INVSQDIST_REPULSE)
    // TODO
#    error ("_getActiveColumnsFromActivationLevelsWithFullLocalInhib not yet implemented for VANILLA_SP_ADD_INVSQDIST_REPULSE")
#  elif defined(VANILLA_SP_ADD_ONE_7x7)
    // TODO
#    error ("_getActiveColumnsFromActivationLevelsWithFullLocalInhib not yet implemented for VANILLA_SP_ADD_KONE_7x7")
#  else
    const uint32* pCurrentReduced = _pReducedActivations;
    for (uint16 uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE; uIndex++, pCurrentReduced++) {
        if (*pCurrentReduced)
            vecOutputIndices.push_back(uIndex);
    }
#  endif
#endif
}
; // template termination

#    ifdef VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onEvaluateBoostingFromColumnUsageWithLocalInhibAlongX() {
    // Stored column-major => use it to our advantage by only computing neighborhood best at columns
    u16fast uXstartOffset = u16fast(_uInhibitionRadius);
    u16fast uXsize = u16fast(_uInhibitionSideSize);
    float fInvNumNeighbors = 1.0f / float(uXsize*VANILLA_HTM_SHEET_HEIGHT);
    uint16* pCurrentBoosting = _pBoostingPerCol;
    const float* pCurrentActivRatio = _pAverageActiveRatioPerColumn;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uXstartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        float fLocalActivityAverage = _getSumFromRange<true, false>(uStartX, uXsize, 0u, VANILLA_HTM_SHEET_HEIGHT,
            _pAverageActiveRatioPerColumn) * fInvNumNeighbors;
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentBoosting++, pCurrentActivRatio++) {
            *pCurrentBoosting = _getBoostFactorUint16(fLocalActivityAverage, *pCurrentActivRatio);
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onEvaluateBoostingFromColumnUsageWithFullLocalInhib() {
    // Full neighborhood computation at each point
    u16fast uStartOffset = size_t(_uInhibitionRadius);
    u16fast uSize = size_t(_uInhibitionSideSize);
    float fInvNumNeighbors = 1.0f / float(uSize*uSize);
    uint16* pCurrentBoosting = _pBoostingPerCol;
    const float* pCurrentActivRatio = _pAverageActiveRatioPerColumn;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uStartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentBoosting++, pCurrentActivRatio++) {
            u16fast uStartY = (uY - uStartOffset) & VANILLA_HTM_SHEET_YMASK;        // wrapping around 32 Y-positions
            float fLocalActivityAverage = _getSumFromRange<true, true>(uStartX, uSize, uStartY, uSize,
                _pAverageActiveRatioPerColumn) * fInvNumNeighbors;
            *pCurrentBoosting = _getBoostFactorUint16(fLocalActivityAverage, *pCurrentActivRatio);
        }
    }
}

#    endif // VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onUpdateOverThresholdRatioTargetWithLocalInhibAlongX() {
    // Stored column-major => use it to our advantage by only computing neighborhood best at columns
    u16fast uXstartOffset = size_t(_uInhibitionRadius);
    u16fast uXsize = size_t(_uInhibitionSideSize);
    float *pCurrentTarget = _pOverThresholdRatioTargetPerColumn;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uXstartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        float fMaxAmongNeighbors = _getMaxFromRange<true, false>(uStartX, uXsize, 0u, VANILLA_HTM_SHEET_HEIGHT,
            _pAverageOverThresholdRatioPerColumn);
        float fTargetThere = fMaxAmongNeighbors * _fOverThresholdTargetVsMaxRatio;
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentTarget++) {
            *pCurrentTarget = fTargetThere;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onUpdateOverThresholdRatioTargetWithFullLocalInhib() {
    // Full neighborhood computation at each point
    u16fast uStartOffset = size_t(_uInhibitionRadius);
    u16fast uSize = size_t(_uInhibitionSideSize);
    float *pCurrentTarget = _pOverThresholdRatioTargetPerColumn;
    for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
        u16fast uStartX = (uX - uStartOffset) & VANILLA_HTM_SHEET_XMASK;        // wrapping around 64 X-positions
        for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, pCurrentTarget++) {
            u16fast uStartY = (uY - uStartOffset) & VANILLA_HTM_SHEET_YMASK;        // wrapping around 32 Y-positions
            float fMaxOverlapDutyCycles = _getMaxFromRange<true, true>(uStartX, uSize, uStartY, uSize,
                _pAverageOverThresholdRatioPerColumn);
            float fTargetThere = fMaxOverlapDutyCycles * _fOverThresholdTargetVsMaxRatio;
            *pCurrentTarget = fTargetThere;
        }
    }
}

#  else // Hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKETS

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bOutputMinActivation>
void VanillaSP::_getActiveColumnsFromActivationLevelsWithBucketInhib(const ActivationLevelType* pActivationLevelsPerCol,
    std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations)
{
    // TODO : if pOutputMinActivations

    u16fast uBucketSize = u16fast(_uBucketSize);
    u16fast uBucketCountY = u16fast(_uBucketCountY);
    u16fast uBucketCountX = uBucketCountY << (VANILLA_HTM_SHEET_SHIFT_DIVX - VANILLA_HTM_SHEET_SHIFT_DIVY);
    u16fast uTableSize = u16fast(_uCurrentWinnerK) + 1u;
    u16fast uStartX = 0u;
    for (u16fast uBucketX = 0u; uBucketX < uBucketCountX; uBucketX++, uStartX += uBucketSize) {
        u16fast uStartY = 0u;
        for (u16fast uBucketY = 0u; uBucketY < uBucketCountY; uBucketY++, uStartY += uBucketSize) {
            u16fast uCountBest = _getBestFromRange<ActivationLevelType, false, false>(
                uStartX, uBucketSize, uStartY, uBucketSize,
                pActivationLevelsPerCol, _pTmpTableBest, uTableSize);
            if (uCountBest) {
                ActivationLevelType uBelowMin = ActivationLevelType(_pTmpTableBest[uCountBest]);
                u16fast uStartIndex = (uStartX << VANILLA_HTM_SHEET_SHIFT_DIVY) + uStartY;
                for (u16fast uX = uStartX, uEndX = uStartX + uBucketSize; uX < uEndX; uX++, uStartIndex += VANILLA_HTM_SHEET_HEIGHT) {
                    u16fast uIndex = uint16(uStartIndex);
                    for (u16fast uY = uStartY, uEndY = uStartY + uBucketSize; uY < uEndY; uY++, uIndex++) {
                        if (pActivationLevelsPerCol[uIndex] > uBelowMin)
                            vecOutputIndices.push_back(uIndex);
                        if (bOutputMinActivation) // static test, shall be optimized out when false
                            pOutputMinActivations[uIndex] = uBelowMin;
                    }
                }
            }
            else if (bOutputMinActivation) { // static test, shall be optimized out when false
                u16fast uStartIndex = (uStartX << VANILLA_HTM_SHEET_SHIFT_DIVY) + uStartY;
                for (u16fast uX = uStartX, uEndX = uStartX + uBucketSize; uX < uEndX; uX++, uStartIndex += VANILLA_HTM_SHEET_HEIGHT) {
                    u16fast uIndex = uint16(uStartIndex);
                    for (u16fast uY = uStartY, uEndY = uStartY + uBucketSize; uY < uEndY; uY++, uIndex++) {
                        pOutputMinActivations[uIndex] = 0u;
                    }
                }
            }
        }
    }
}
; // template termination

#    ifdef VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onEvaluateBoostingFromColumnUsageWithBucketInhib() {
    u16fast uBucketSize = u16fast(_uBucketSize);
    u16fast uBucketCountY = u16fast(_uBucketCountY);
    u16fast uBucketCountX = uBucketCountY << (VANILLA_HTM_SHEET_SHIFT_DIVX - VANILLA_HTM_SHEET_SHIFT_DIVY);
    float fInvNumNeighbors = 1.0f / float(_uBucketSize * _uBucketSize);
    u16fast uStartX = 0u;
    for (u16fast uBucketX = 0u; uBucketX < uBucketCountX; uBucketX++, uStartX += uBucketSize) {
        u16fast uStartY = 0u;
        for (u16fast uBucketY = 0u; uBucketY < uBucketCountY; uBucketY++, uStartY += uBucketSize) {
            float fLocalActivityAverage = _getSumFromRange<false, false>(uStartX, uBucketSize, uStartY, uBucketSize,
                _pAverageActiveRatioPerColumn) * fInvNumNeighbors;
            u16fast uStartIndex = (uStartX << VANILLA_HTM_SHEET_SHIFT_DIVY) + uStartY;
            for (u16fast uX = uStartX, uEndX = uStartX + uBucketSize; uX < uEndX; uX++, uStartIndex += VANILLA_HTM_SHEET_HEIGHT) {
                uint16* pCurrentBoosting = _pBoostingPerCol + uStartIndex;
                const float* pCurrentActivRatio = _pAverageActiveRatioPerColumn + uStartIndex;
                for (u16fast uY = uStartY, uEndY = uStartY + uBucketSize; uY < uEndY;
                        uY++, pCurrentBoosting++, pCurrentActivRatio++) {
                    *pCurrentBoosting = _getBoostFactorUint16(fLocalActivityAverage, *pCurrentActivRatio);
                }
            }
        }
    }
}

#    endif // VANILLA_SP_USE_BOOSTING

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onUpdateOverThresholdRatioTargetWithBucketInhib() {
    u16fast uBucketSize = u16fast(_uBucketSize);
    u16fast uBucketCountY = u16fast(_uBucketCountY);
    u16fast uBucketCountX = uBucketCountY << (VANILLA_HTM_SHEET_SHIFT_DIVX - VANILLA_HTM_SHEET_SHIFT_DIVY);
    u16fast uStartX = 0u;
    for (u16fast uBucketX = 0u; uBucketX < uBucketCountX; uBucketX++, uStartX += uBucketSize) {
        u16fast uStartY = 0u;
        for (u16fast uBucketY = 0u; uBucketY < uBucketCountY; uBucketY++, uStartY += uBucketSize) {
            float fMaxAmongNeighbors = _getMaxFromRange<false, false>(uStartX, uBucketSize, uStartY, uBucketSize,
                _pAverageOverThresholdRatioPerColumn);
            float fTargetThere = fMaxAmongNeighbors * _fOverThresholdTargetVsMaxRatio;
            u16fast uStartIndex = (uStartX << VANILLA_HTM_SHEET_SHIFT_DIVY) + uStartY;
            for (u16fast uX = uStartX, uEndX = uStartX + uBucketSize; uX < uEndX; uX++, uStartIndex += VANILLA_HTM_SHEET_HEIGHT) {
                float *pCurrentTarget = _pOverThresholdRatioTargetPerColumn + uStartIndex;
                for (u16fast uY = uStartY, uEndY = uStartY + uBucketSize; uY < uEndY; uY++, pCurrentTarget++) {
                    *pCurrentTarget = fTargetThere;
                }
            }
        }
    }
}

#  endif // valueof VANILLA_SP_USE_LOCAL_INHIB

#endif // VANILLA_SP_USE_LOCAL_INHIB

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType, bool bOutputMinActivation>
void VanillaSP::_getActiveColumnsFromActivationLevelsWithGlobalInhib(const ActivationLevelType* pActivationLevelsPerCol,
    std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations)
{
    u16fast uCountBest = _getBestFromRange<ActivationLevelType, false, false>(
        0u, VANILLA_HTM_SHEET_WIDTH, 0u, VANILLA_HTM_SHEET_HEIGHT,
        pActivationLevelsPerCol, _pTmpTableBest, u16fast(_uCurrentWinnerK)+1u);
    if (uCountBest) {
        ActivationLevelType uBelowMin = ActivationLevelType(_pTmpTableBest[uCountBest]);
        u16fast uIndex = 0u;
        const ActivationLevelType* pCurrentActivationLevel = pActivationLevelsPerCol;
        for (u16fast uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++) {
            for (u16fast uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++, uIndex++, pCurrentActivationLevel++) {
                if (*pCurrentActivationLevel > uBelowMin)
                    vecOutputIndices.push_back(uIndex);
            }
        }
        if (bOutputMinActivation) { // static test, shall be optimized out when false
            for (uint32* pCurrentOutputMin = pOutputMinActivations, *pEnd = pOutputMinActivations + VANILLA_HTM_SHEET_2DSIZE;
                pCurrentOutputMin < pEnd; pCurrentOutputMin++) {
                *pCurrentOutputMin = uint32(uBelowMin);
            }
        }
    } else if (bOutputMinActivation) { // static test, shall be optimized out when false
        for (uint32* pCurrentOutputMin = pOutputMinActivations, *pEnd = pOutputMinActivations + VANILLA_HTM_SHEET_2DSIZE;
            pCurrentOutputMin < pEnd; pCurrentOutputMin++) {
            *pCurrentOutputMin = 0u;
        }
    }
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onUpdateOverThresholdRatioTargetWithGlobalInhib()
{
    float fMaxAmongNeighbors = _getMaxFromRange<false, false>(0u, VANILLA_HTM_SHEET_WIDTH, 0u, VANILLA_HTM_SHEET_HEIGHT,
        _pAverageOverThresholdRatioPerColumn);
    float fConstTargetNow = fMaxAmongNeighbors * _fOverThresholdTargetVsMaxRatio;
    for (float *pCurrentTarget = _pOverThresholdRatioTargetPerColumn,
            *pEnd = _pOverThresholdRatioTargetPerColumn + VANILLA_HTM_SHEET_2DSIZE;
            pCurrentTarget < pEnd; pCurrentTarget++) {
        *pCurrentTarget = fConstTargetNow;
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
template<typename ActivationLevelType>
void VanillaSP::_getActiveColumnsFromActivationLevels(const ActivationLevelType* pActivationLevelsPerCol,
    std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations)
{
#if defined(VANILLA_SP_USE_LOCAL_INHIB)
#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
    if (_uInhibitionSideSize < VANILLA_SP_MIN_AREA_SIDE_SIZE || _uInhibitionSideSize >= VANILLA_HTM_SHEET_WIDTH) {
        if (pOutputMinActivations)
            _getActiveColumnsFromActivationLevelsWithGlobalInhib<ActivationLevelType, true>(pActivationLevelsPerCol,
                vecOutputIndices, pOutputMinActivations);
        else
            _getActiveColumnsFromActivationLevelsWithGlobalInhib<ActivationLevelType, false>(pActivationLevelsPerCol,
                vecOutputIndices, 0);
    } else if (_uInhibitionSideSize >= VANILLA_HTM_SHEET_HEIGHT) {
        if (pOutputMinActivations)
            _getActiveColumnsFromActivationLevelsWithLocalInhibAlongX<ActivationLevelType, true>(pActivationLevelsPerCol,
                vecOutputIndices, pOutputMinActivations);
        else
            _getActiveColumnsFromActivationLevelsWithLocalInhibAlongX<ActivationLevelType, false>(pActivationLevelsPerCol,
                vecOutputIndices, 0);
    } else {
        if (pOutputMinActivations)
            _getActiveColumnsFromActivationLevelsWithFullLocalInhib<ActivationLevelType, true>(pActivationLevelsPerCol,
                vecOutputIndices, pOutputMinActivations);
        else
            _getActiveColumnsFromActivationLevelsWithFullLocalInhib<ActivationLevelType, false>(pActivationLevelsPerCol,
                vecOutputIndices, 0);
    }
#  else // hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
    if (pOutputMinActivations)
        _getActiveColumnsFromActivationLevelsWithBucketInhib<ActivationLevelType, true>(pActivationLevelsPerCol,
            vecOutputIndices, pOutputMinActivations);
    else
        _getActiveColumnsFromActivationLevelsWithBucketInhib<ActivationLevelType, false>(pActivationLevelsPerCol,
            vecOutputIndices, 0);
#  endif
#else   // Global inhib
    if (pOutputMinActivations)
        _getActiveColumnsFromActivationLevelsWithGlobalInhib<ActivationLevelType, true>(pActivationLevelsPerCol,
            vecOutputIndices, pOutputMinActivations);
    else
        _getActiveColumnsFromActivationLevelsWithGlobalInhib<ActivationLevelType, false>(pActivationLevelsPerCol,
            vecOutputIndices, 0);
#endif
}
; // template termination

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_updateSynapsesOnActiveColumnsTowardsCurrentInput(const uint64* pInputBinaryBitmap,
    const std::vector<uint16>& vecActiveIndices)
{
    for (auto itActive = vecActiveIndices.begin(), itEndActive = vecActiveIndices.end(); itActive != itEndActive; itActive++) {
        uint16 uActiveIndex = *itActive;
        Segment& currentSeg = _pSegments[uActiveIndex];
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
        uint64* pCurrentConnectivityField = _pConnectivityFields + _uConnectivityFieldsQwordSizePerColumn * uActiveIndex;
#endif
        u16fast uCount = currentSeg._uCount;
        uint16* pPreSyn = currentSeg._tPreSynIndex;
        VANILLA_SP_SYN_PERM_TYPE* pPerm = currentSeg._tPermValue;
        for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
            u16fast uPreSynCellIndex = *pPreSyn;
            u16fast uPreSynCellQword = uPreSynCellIndex >> 6u;
            u16fast uPreSynCellBit = uPreSynCellIndex & 0x003Fu;
            VANILLA_SP_SYN_PERM_TYPE permanenceValue = *pPerm;
            uint64 uPreSynCellValue = (pInputBinaryBitmap[uPreSynCellQword] >> uPreSynCellBit) & 1uLL;
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
            // If we're using connectivity field optimization,
            //   then we need to update here the connectivity bitfield whenever a synapse connection status changes as
            //   a result of the changes to its permanence value.
            if (uPreSynCellValue) {
                if (permanenceValue < VANILLA_SP_SYN_CONNECTED_PERM) {
                    permanenceValue = _increasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_ACTIVE_INC);
                    if (permanenceValue >= VANILLA_SP_SYN_CONNECTED_PERM) {
                        uint64 uPreSynCellMask = 1uLL << uPreSynCellBit;
                        pCurrentConnectivityField[uPreSynCellQword] |= uPreSynCellMask;
                    }
                } else {
                    permanenceValue = _increasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_ACTIVE_INC);
                }
            } else {
                if (permanenceValue >= VANILLA_SP_SYN_CONNECTED_PERM) {
                    permanenceValue = _decreasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_INACTIVE_DEC);
                    if (permanenceValue < VANILLA_SP_SYN_CONNECTED_PERM) {
                        uint64 uPreSynCellMask = ~(1uLL << uPreSynCellBit);
                        pCurrentConnectivityField[uPreSynCellQword] &= uPreSynCellMask;
                    }
                } else {
                    permanenceValue = _decreasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_INACTIVE_DEC);
                }
            }
            *pPerm = permanenceValue;
#else // !VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
            VANILLA_SP_SYN_SIGNED_PERM_TYPE ifPreSynActive = VANILLA_SP_SYN_SIGNED_PERM_TYPE(uPreSynCellValue);
            VANILLA_SP_SYN_SIGNED_PERM_TYPE ifPreSynSilent = VANILLA_SP_SYN_SIGNED_PERM_TYPE(1) - ifPreSynActive;
            VANILLA_SP_SYN_SIGNED_PERM_TYPE permanenceChange =
                ifPreSynActive * VANILLA_SP_SYN_SIGNED_PERM_TYPE(VANILLA_SP_SYN_PERM_ACTIVE_INC) +
                ifPreSynSilent * VANILLA_SP_SYN_SIGNED_PERM_TYPE(VANILLA_SP_SYN_PERM_INACTIVE_DEC);
            *pPerm = _updatePermanence(permanenceValue, permanenceChange);
#endif
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onIncreasePermanencesForUnderUsedColums()
{
    const float* pCurrentAverageOverThresholdRatio = _pAverageOverThresholdRatioPerColumn;
    const float* pCurrentOverThresholdRatioTarget = _pOverThresholdRatioTargetPerColumn;
    Segment* pCurrentSegment = _pSegments;
    for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE;
            uIndex++, pCurrentAverageOverThresholdRatio++, pCurrentOverThresholdRatioTarget++, pCurrentSegment++) {
        if (*pCurrentAverageOverThresholdRatio < *pCurrentOverThresholdRatioTarget) {
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
            uint64* pCurrentConnectivityField = _pConnectivityFields + _uConnectivityFieldsQwordSizePerColumn * uIndex;
#endif
            u16fast uCount = pCurrentSegment->_uCount;
            const uint16* pPreSyn = pCurrentSegment->_tPreSynIndex;
            VANILLA_SP_SYN_PERM_TYPE* pPerm = pCurrentSegment->_tPermValue;
            for (u16fast uSyn = 0u; uSyn < uCount; uSyn++, pPreSyn++, pPerm++) {
                VANILLA_SP_SYN_PERM_TYPE permanenceValue = *pPerm;
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
                // If we're using connectivity field optimization,
                //   then we need to add here to the connectivity bitfield whenever an unconnected synapse
                //   becomes connected as a result of the increase applied to its permanence value.
                if (permanenceValue < VANILLA_SP_SYN_CONNECTED_PERM) {
                    permanenceValue = _increasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_BELOW_STIM_INC);
                    if (permanenceValue >= VANILLA_SP_SYN_CONNECTED_PERM) {
                        u16fast uPreSynCellIndex = *pPreSyn;
                        u16fast uPreSynCellQword = uPreSynCellIndex >> 6u;
                        u16fast uPreSynCellBit = uPreSynCellIndex & 0x003Fu;
                        uint64 uPreSynCellMask = 1uLL << uPreSynCellBit;
                        pCurrentConnectivityField[uPreSynCellQword] |= uPreSynCellMask;
                    }
                } else {
                    permanenceValue = _increasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_BELOW_STIM_INC);
                }
                *pPerm = permanenceValue;
#else  // !VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
                *pPerm = _increasePermanence(permanenceValue, VANILLA_SP_SYN_PERM_BELOW_STIM_INC);
#endif
            }
        }
    }
}

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
void VanillaSP::_onEvaluateColumnUsage(const uint16* pRawActivationLevelsPerCol, const uint64* pResultingBinaryBitmap)
{
    memset((void*)_pTmpBinaryOverThresholdActivations, 0, VANILLA_HTM_SHEET_BYTES_BINARY);
    const uint16* pCurrentRawActivationLevel = pRawActivationLevelsPerCol;
    for (u16fast uIndex = 0u; uIndex < VANILLA_HTM_SHEET_2DSIZE; uIndex++, pCurrentRawActivationLevel++) {
        if (*pCurrentRawActivationLevel >= VANILLA_SP_DEFAULT_STIMULUS_THRESHOLD) {
            u16fast uQword = uIndex >> 6u;
            u16fast uBit = uIndex & 0x003Fu;
            _pTmpBinaryOverThresholdActivations[uQword] |= (1uLL << uBit);
        }
    }
    _integrateBinaryFieldToMovingAverages(_pAverageOverThresholdRatioPerColumn, _pTmpBinaryOverThresholdActivations,
        _uColumnUsageIntegrationWindow);
    _integrateBinaryFieldToMovingAverages(_pAverageActiveRatioPerColumn, pResultingBinaryBitmap,
        _uColumnUsageIntegrationWindow);
}

#if defined(VANILLA_SP_SUBNAMESPACE)
    } // namespace VANILLA_SP_SUBNAMESPACE
#endif
} // namespace HTMATCH

