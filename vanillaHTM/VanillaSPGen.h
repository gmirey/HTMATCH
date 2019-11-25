/* -----------------------------------
 * HTMATCH
 * VanillaSPGen.h
 * -----------------------------------
 * Expands VANILLA_SP* configuration choices to second-order options, then declares the VanillaSP class
 *   which implements a vanilla-HTM-like Spatial Pooler
 * Warning : No multi-inclusion guard !!!
 *   This file is intended to be included multiple times with different config options indeed, if the user wishes so.
 *      (to have multiple declared versions, you may #define VANILLA_SP_SUBNAMESPACE to an identifier of your choice) 
 *   => Prefer including "VanillaSP.h" as a bread-and butter, classical header, if you'd rather forfeit this option
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
// Undefining every second-order config option for the Spatial Pooler beforehand, if need be
//   (we can thus include same header multiple times, and run different versions at the same time,
//     for example by defining VANILLA_SP_SUBNAMESPACE to an identifier of your choice) 

#ifdef VANILLA_SP_USE_LOCAL_INHIB
#  undef VANILLA_SP_USE_LOCAL_INHIB
#endif
#ifdef VANILLA_SP_USE_BOOSTING
#  undef VANILLA_SP_USE_BOOSTING
#endif
#ifdef VANILLA_SP_UPDATERAD_KIND
#  undef VANILLA_SP_UPDATERAD_KIND
#endif
#ifdef VANILLA_SP_UPDATERAD_OPTIM
#  undef VANILLA_SP_UPDATERAD_OPTIM
#endif
#ifdef VANILLA_SP_SYN_PERM_TYPE
#  undef VANILLA_SP_SYN_PERM_TYPE
#endif
#ifdef VANILLA_SP_SYN_SIGNED_PERM_TYPE
#  undef VANILLA_SP_SYN_SIGNED_PERM_TYPE
#endif
#ifdef VANILLA_SP_SYN_PERM_INACTIVE_DEC
#  undef VANILLA_SP_SYN_PERM_INACTIVE_DEC
#endif
#ifdef VANILLA_SP_SYN_PERM_ACTIVE_INC
#  undef VANILLA_SP_SYN_PERM_ACTIVE_INC
#endif
#ifdef VANILLA_SP_SYN_CONNECTED_PERM
#  undef VANILLA_SP_SYN_CONNECTED_PERM
#endif
#ifdef VANILLA_SP_SYN_PERM_BELOW_STIM_INC
#  undef VANILLA_SP_SYN_PERM_BELOW_STIM_INC
#endif
#ifdef VANILLA_SP_SYN_PERM_TYPE_MAX
#  undef VANILLA_SP_SYN_PERM_TYPE_MAX
#endif

#include "VanillaHTMConfig.h"

// NOT an inclusion guard per se, however can be queried by calling code :)
//   (For IDEs to be happy parsing cpp-like-but-.h files)
#ifndef _VANILLA_SP_GEN_H
#  define _VANILLA_SP_GEN_H
#endif

// - - - - - - - - - - - - - - - - - - - -
// Defaulting to some VANILLA_SP_CONFIG and VANILLA_SP_RUNTIME values if need be

#ifndef VANILLA_SP_CONFIG
#  define VANILLA_SP_CONFIG            VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING
#endif

#ifndef VANILLA_SP_SYNAPSE_KIND
#  define VANILLA_SP_SYNAPSE_KIND    VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32
#endif

// - - - - - - - - - - - - - - - - - - - -
// Now setting up configurations based upon current VANILLA_SP_CONFIG value

#if       (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING)
//          NOOP
#elif     (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_BUCKET_NOBOOSTING)
#           define VANILLA_SP_USE_LOCAL_INHIB        VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
#elif     (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL_NOBOOSTING)
#           define VANILLA_SP_USE_LOCAL_INHIB        VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL
#else
// always use boosting in the following
#  define VANILLA_SP_USE_BOOSTING                    VANILLA_SP_BOOSTING_VALUE_SWEET      
#  if     (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_GLOBAL)
//            NOOP
#  elif   (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_BUCKET)
#           define VANILLA_SP_USE_LOCAL_INHIB        VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET
#  else
//   always use nominal local inhib in the following
#    define  VANILLA_SP_USE_LOCAL_INHIB                VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL
#    if   (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL)
//          NOOP
#    elif (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL_CORRECTEDUPDATERAD)
#            define VANILLA_SP_UPDATERAD_KIND        VANILLA_SP_UPDATERAD_KIND_CONST_CORRECTED
#    elif (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD)
#            define VANILLA_SP_UPDATERAD_KIND        VANILLA_SP_UPDATERAD_KIND_CONST_NOUPDATE
#    elif (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_ALL_OPTIM)
#            error "VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_ALL_OPTIM not fully implemented yet" // TODO
#            define VANILLA_SP_UPDATERAD_KIND        VANILLA_SP_UPDATERAD_KIND_CONST_NOUPDATE
#            define VANILLA_SP_NEIGHBORHOOD_OPTIM    VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_ALGORITHM
#    elif (VANILLA_SP_CONFIG == VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_GAUSSTEST)
#            error "VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_GAUSSTEST not fully implemented yet" // TODO
#            define VANILLA_SP_UPDATERAD_KIND        VANILLA_SP_UPDATERAD_KIND_CONST_NOUPDATE
#            define VANILLA_SP_NEIGHBORHOOD_OPTIM    VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSTEST
#    else
#      error "Unhandled value for VANILLA_SP_CONFIG"
#    endif
#  endif
#endif

// - - - - - - - - - - - - - - - - - - - -
// ...and setting up configurations based upon current VANILLA_SP_SYNAPSE_KIND values

#if (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32)
#  define VANILLA_SP_SYN_PERM_TYPE                  float           // obviously...
#  define VANILLA_SP_SYN_SIGNED_PERM_TYPE           float           // how neat is that?
#  define VANILLA_SP_SYN_PERM_INACTIVE_DEC          0.01f           // instead of nupic default 0.008
#  define VANILLA_SP_SYN_PERM_ACTIVE_INC            0.06666667f     // instead of nupic default 0.05
#  define VANILLA_SP_SYN_CONNECTED_PERM             0.13333333f     // instead of nupic default 0.1
#  define VANILLA_SP_SYN_PERM_BELOW_STIM_INC        0.016666667f    // one eigth, instead of nupic one tenth of connected
#  define VANILLA_SP_SYN_PERM_TYPE_MAX              1.0             // shall never be crossed
#elif (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16)
#  define VANILLA_SP_SYN_PERM_TYPE                  uint16          // 16b fixed point [0..65535] representing [0.0 .. 1.0]
#  define VANILLA_SP_SYN_SIGNED_PERM_TYPE           int32           // we'll compute a few things on signed int32 before
                                                                    //   casting back to uint16
#  define VANILLA_SP_SYN_PERM_INACTIVE_DEC          655u            // quite precisely the 0.01 above
#  define VANILLA_SP_SYN_PERM_ACTIVE_INC            4369u           // quite precisely the 0.06667 above
#  define VANILLA_SP_SYN_CONNECTED_PERM             8738u           // quite precisely the 0.13333 above
#  define VANILLA_SP_SYN_PERM_BELOW_STIM_INC        1092u           // quite precisely the 0.016667 above
#  define VANILLA_SP_SYN_PERM_TYPE_MAX              65535           // shall never be crossed, and represents 1.0
#elif (VANILLA_SP_SYNAPSE_KIND == VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8)
#  define VANILLA_SP_SYN_PERM_TYPE                  uint8           // 8b fixed point [0..255] representing [0.0 .. 1.0]
#  define VANILLA_SP_SYN_SIGNED_PERM_TYPE           int32           // we'll compute a few things on signed int32 before
                                                                    //   casting back to uint8
#  define VANILLA_SP_SYN_PERM_INACTIVE_DEC          2u              // very roughly the 0.01 above
#  define VANILLA_SP_SYN_PERM_ACTIVE_INC            17u             // quite precisely the 0.06667 above
#  define VANILLA_SP_SYN_CONNECTED_PERM             34u             // quite precisely the 0.13333 above
#  define VANILLA_SP_SYN_PERM_BELOW_STIM_INC        4u              // close to the 0.016667 above
#  define VANILLA_SP_SYN_PERM_TYPE_MAX              255             // shall never be crossed, and represents 1.0
#else
#  error "no permanence values were adjusted for this value of VANILLA_SP_SYNAPSE_KIND"
#endif

#include "tools/sdr.h"

namespace HTMATCH {
#if defined(VANILLA_SP_SUBNAMESPACE)
    namespace VANILLA_SP_SUBNAMESPACE {
#endif

// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -
// The 'Vanilla' Spatial Pooler class definition, at last!
// - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - -
// VanillaSP class
// - - - - - - - - - - - - - - - - - - - -
// HTM Spatial Pooler is in charge of converting any input to a sparsely encoded representation,
//   while providing semantic overlap, optimal distribution of activations, as well as learning
//   ability towards best-fit for actually seen inputs.
// Input should optimally already be quite sparse, and its overlapping bits should have some
//   amount of overlapping semantics, such as when emitted by an HTM encoder.
// However, the Spatial Pooler will do its best to enforce a given sparsity, and will try to give each
//   column some actual informational value equitably, based on learned inputs. It is moreover in best
//   spot for coercing various encodings to comparable dimensions than the Temporal Memory class, which
//   usually uses its output.
// Also, the Spatial Pooler models the biological "proximal synapses" of several cells
//   in a single cortical minicolumn, as theorized by HTM.
// - - - - - - - - - - - - - - - - - - - -
class VanillaSP {
public:

    // Only available Ctor
    VanillaSP(
        // @nupic.core: inputDimensions
        uint8 uNumberOfInputSheets,                     // now fixed to this multiple of 64x32
                                                        //   (VANILLA_HTM_SHEET_WIDTH x VANILLA_HTM_SHEET_HEIGHT).
                                                        //   acceptable [1..32]  

        // @nupic.core: columnDimensions                // now fixed to VANILLA_HTM_SHEET_WIDTH x VANILLA_HTM_SHEET_HEIGHT (64x32)

        // @nupic.core: potentialRadius
        uint8 uPotentialConnectivityRadius = VANILLA_SP_DEFAULT_POTENTIAL_RADIUS,   // different default, otherwise nearly similar (only in 2D, and only along width if >15)

        // @nupic.core: potentialPct
        float fPotentialConnectivityRatio = VANILLA_SP_DEFAULT_POTENTIAL_RATIO,     // 'ratio' since this is not a percentage...

        // @nupic.core: globalInhibition                // now a compilation option

        // @nupic.core: localAreaDensity
        float fActivationDensityRatio = VANILLA_SP_DEFAULT_DENSITY_RATIO,           // now used as 'the' default method
                                                                                    //   to compute the 'K' of K-winners

        // @nupic.core: numActiveColumnsPerInhArea      // now discared, fDensityRatio is always used instead

        // @nupic.core: stimulusThreshold               // now a somewhat fixed compilation param

        // @nupic.core: synPermInactiveDec              // now a compilation param, dependent upon permanence type
        // @nupic.core: synPermActiveInc                // now a compilation param, dependent upon permanence type
        // @nupic.core: synPermConnected                // now a compilation param, dependent upon permanence type

        // @nupic.core: minPctOverlapDutyCycles
        float fOverThresholdTargetVsMaxRatio = VANILLA_SP_DEFAULT_TARGET_VS_MAX_RATIO, // 'ratio' since this is not a percentage

        // @nupic.core: dutyCyclePeriod
        uint64 uColumnUsageIntegrationWindow = VANILLA_SP_DEFAULT_INTEGRATION_WINDOW,   // same semantic as nupic version

        // @nupic.core: boostStrength                   // now a compilation option & param

        // @nupic.core: seed
        uint64 uSeed = VANILLA_SP_DEFAULT_SEED          // different default, otherwise nearly similar.
                                                        //   Note that random generator is only used during initialization.

        // @nupic.core: spVerbosity                     // n/a
        // @nupic.core: wrapAround                      // now always assumed true
    );

    // Dtor...
    ~VanillaSP();

    // - - - - - - - - - - - - - - - - - - - -
    // Bread and butter "compute" method, similar to vanilla HTM spatial pooler...
    // Nb: Input Indices shall be col-major, z-minor => index 35 is (x=1;y=3;z=0). Output indices will be col-major too.
    // Optional: if non-null, pOutputBinaryBitmap will be filled with same info as 'vecOutputIndices',
    //   however in bitfield-form, for no additional overhead.
    // - - - - - - - - - - - - - - - - - - - -
    void compute(const std::vector<uint16>& vecInputIndices, std::vector<uint16>& vecOutputIndices, bool bLearning = true,
        uint64* pOutputBinaryBitmap = 0, uint32* pOutputMinActivations = 0) {
        SDRTools::toBinaryBitmap64(vecInputIndices, _pTmpBinaryInputBuffer,
            size_t(_uInputSheetsCount) * VANILLA_HTM_SHEET_BYTES_BINARY);
        _compute(_pTmpBinaryInputBuffer, vecOutputIndices, bLearning, pOutputBinaryBitmap, pOutputMinActivations);
    }

    // - - - - - - - - - - - - - - - - - - - -
    // Alternate signature for the "compute" method, for whenever we already have an input presented in bitfield-form
    //   (instead of a vector of active indices).
    // - - - - - - - - - - - - - - - - - - - -
    void compute(const uint64* pInputBinaryBitmap, std::vector<uint16>& vecOutputIndices, bool bLearning = true,
        uint64* pOutputBinaryBitmap = 0, uint32* pOutputMinActivations = 0) {
        _compute(pInputBinaryBitmap, vecOutputIndices, bLearning, pOutputBinaryBitmap, pOutputMinActivations);
    }

    // - - - - - - - - - - - - - - - - - - - -
    // Returns the raw activation levels (number of active presynaptic cells) which were used at previous call of 'compute'.
    //   results are presented col-major across the 2048 minicolumns
    // - - - - - - - - - - - - - - - - - - - -
    const uint16* getRawActivationLevels() const { return _pTmpRawActivationLevelsPerCol; }

    // - - - - - - - - - - - - - - - - - - - -
    // Returns the boosted activation levels (raw times fixPt 'boost' value, 8b after point => 256 represents 1.0),
    //   which were used at previous call of 'compute' results are presented col-major across the 2048 minicolumns
    // Warning: May return null if boosting ain't specified for this implementation
    // - - - - - - - - - - - - - - - - - - - -
    const uint32* getBoostedActivationLevels() const {
#ifdef VANILLA_SP_USE_BOOSTING
        return _pTmpBoostedActivationLevelsPerCol;
#else
        return 0;
#endif
    }

    // - - - - - - - - - - - - - - - - - - - -
    // Returns the boost factors as uint16 fixed point values, 8b after point (=> 256 represents 1.0),
    //   which were used at previous call of 'compute'. Results are presented col-major across the 2048 minicolumns
    // Warning: May return null if boosting ain't specified for this implementation
    // - - - - - - - - - - - - - - - - - - - -
    const uint16* getBoostingFactors() const {
#ifdef VANILLA_SP_USE_BOOSTING
        return _pBoostingPerCol;
#else
        return 0;
#endif
    }

    // - - - - - - - - - - - - - - - - - - - -
    // Returns the inhibition radius which is to be used at next call of 'compute',
    //   possibly updated dynamically when calling compute() itself.
    // - - - - - - - - - - - - - - - - - - - -
    uint8 getInhibitionRadius() const { return _uInhibitionRadius; }

    // - - - - - - - - - - - - - - - - - - - -
    // Returns the total inhibition size (of the side of a 2D square), straightforward from inhibition radius
    // - - - - - - - - - - - - - - - - - - - -
    uint8 getInhibitionSideSize() const { return _uInhibitionSideSize; }

    // - - - - - - - - - - - - - - - - - - - -
    // Defines the 'segment' structure held by each minicolumn,
    //   biologically representing 'proximal' parts of dendrites in case of the SP.
    // - - - - - - - - - - - - - - - - - - - -
    struct Segment {
        // Actual number of all potential synapses defined on this segment, having been chosen to reach
        //   'fPotentialConnectivityRatio' among all from the neighboring connectivity range (dependent upon
        //   'uPotentialConnectivityRadius', and spanning across all 'uNumberOfInputSheets')
        uint16 _uCount;                                                         

        // Table of pre-synaptic cell indices, for each of the potential synapse (col-major, z-minor)
        uint16 _tPreSynIndex[VANILLA_SP_MAX_SYNAPSES_PER_SEG];

        // Table of current permanence values. This is the heart of the dynamic part of the model, and where most of the
        //   learning ability reside. Once a permanence value reaches or exceeds 'VANILLA_SP_SYN_CONNECTED_PERM', the synapse is
        //   considered 'connected', and will then be allowed to take into account the activity of the associated presynaptic
        //   cell on each round, when it is time to compute the current activation level of the segment.
        VANILLA_SP_SYN_PERM_TYPE _tPermValue[VANILLA_SP_MAX_SYNAPSES_PER_SEG];
    };

    // - - - - - - - - - - - - - - - - - - - -
    // Static helper methods to auto-report about configuration choices for a particular class
    // ... just in case we indeed defined many of them
    // - - - - - - - - - - - - - - - - - - - -

    typedef VANILLA_SP_SYN_PERM_TYPE SynPermanenceType;
    static VANILLA_SP_SYN_PERM_TYPE getMaxSynPermanence() { return VANILLA_SP_SYN_PERM_TYPE_MAX; }
    static VANILLA_SP_SYN_PERM_TYPE getConnectedSynPermanence() { return VANILLA_SP_SYN_CONNECTED_PERM; }
    static int getConfigIndex() { return VANILLA_SP_CONFIG; }
    static int getSynapseKindIndex() { return VANILLA_SP_SYNAPSE_KIND; }
    static bool doesUseConnectivityFieldOpti() {
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
        return true;
#else
        return false;
#endif
    }
    static bool doesUseBoosting() {
#ifdef VANILLA_SP_USE_BOOSTING
        return true;
#else
        return false;
#endif
    }
    static bool doesUseBucketInhib() {
#ifdef VANILLA_SP_USE_LOCAL_INHIB
#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET)
        return true;
#  else
        return false;
#  endif
#else
        return false;
#endif
    }
    static bool doesUseLocalInhib() {
#ifdef VANILLA_SP_USE_LOCAL_INHIB
#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)
        return true;
#  else
        return false;
#  endif
#else
        return false;
#endif
    }
    static int getUpdateRadIndex() {
#ifdef VANILLA_SP_UPDATERAD_KIND
        return VANILLA_SP_UPDATERAD_KIND;
#else
        return 0;
#endif
    }
    static int getNeighborhoodOptimIndex() {
#ifdef VANILLA_SP_NEIGHBORHOOD_OPTIM
        return VANILLA_SP_NEIGHBORHOOD_OPTIM;
#else
        return 0;
#endif
    }

    // *** *** *** *** *** *** *** *** *** ***
    // *** *** *** *** *** *** *** *** *** ***
private:
    // Behold ! Do not stare beyond this point !
    // - - - - - - - - - - - - - - - - - - - -
    // Okay, well... you've been warned...
    // *** *** *** *** *** *** *** *** *** ***
    // *** *** *** *** *** *** *** *** *** ***

    // Implements the two variants of the public 'compute' interface above, in same way.
    //   Yes, it means the implementation prefer brute-force bitfield inputs.
    void _compute(const uint64* pInputBinaryBitmap, std::vector<uint16>& vecOutputIndices, bool bLearning,
        uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations);

    // Will compute the initial (unihibited) activation levels for each colums, based on current input and current state of
    //   synaptic connections to them (Working against bitfield input)
    void _computeUnrestrictedActivationLevels(const uint64* pInputBinaryBitmap, uint16* pOutputActivationLevelsPerCol) const;

#ifdef VANILLA_SP_USE_BOOSTING

    // Implements the bulk of the _compute() method once raw activation levels have been computed, when use_boosting config option is on
    void _computeActiveColumnsAndLearnWhenBoosted(const uint64* pInputBinaryBitmap, std::vector<uint16>& vecOutputIndices,
        bool bLearning, uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations);

    // Will compute the initial, yet "boosted" activation levels. That is, activation levels multiplied by boost factors
    //   we use 16b fixed point here, 8b after point 'boost' values. And 32b integer results (=> also 8b after point fixpts)
    void _computeBoostedActivationLevels(const uint16* pActivationLevelsPerCol,
        uint32* pOutputBoostedActivationLevelsPerCol) const;

    // Will compute brand new boost factors for each column based on column usage estimation relative to neighborhood
    //   we use 16b fixed point here, 8b after point 'boost' values.
    // Note: Disabled: direct call to the various distinct implementations performed on '_compute'
    // void _onEvaluateBoostingFromColumnUsage();

    // implements _onEvaluateBoostingFromColumnUsage() when global inhibition is selected
    void _onEvaluateBoostingFromColumnUsageWithGlobalInhib();

#else // !VANILLA_SP_USE_BOOSTING

    // Implements the bulk of the _compute() method once raw activation levels have been computed, when use_boosting config option is off
    void _computeActiveColumnsAndLearnWhenNoBoosting(const uint64* pInputBinaryBitmap, std::vector<uint16>& vecOutputIndices,
        bool bLearning, uint64* pOutputBinaryBitmap, uint32* pOutputMinActivations);

#endif // VANILLA_SP_USE_BOOSTING

    // Will select the winning, 'active' columns on this round from either raw or boosted activation levels
    //   ('ActivationLevelType' will discriminate between the two), relative to neighborhood, by chosing a total number of active
    //   columns equal (or hopefully close to) fActivationDensityRatio * VANILLA_HTM_SHEET_2DSIZE
    template<typename ActivationLevelType>
    void _getActiveColumnsFromActivationLevels(const ActivationLevelType* pActivationLevelsPerCol,
        std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations) const;

#ifdef VANILLA_SP_USE_LOCAL_INHIB

    // May update the inhibition range dynamically, depending on the inhibition radius options
    //   @see the 'updateInhibitionRadius' discussion as presented in VanillaHTMConfig.h
    void _onUpdateDynamicInhibitionRange();

#  if (VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL)

    // implements _getActiveColumnsFromActivationLevels() when local inhib can be computed along x coordinates only
    template<typename ActivationLevelType, bool bOutputMinActivation>
    void _getActiveColumnsFromActivationLevelsWithLocalInhibAlongX(const ActivationLevelType* pActivationLevelsPerCol,
        std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations) const;

    // implements _getActiveColumnsFromActivationLevels() when local inhib requires full-blown neighborhood per column
    template<typename ActivationLevelType, bool bOutputMinActivation>
    void _getActiveColumnsFromActivationLevelsWithFullLocalInhib(const ActivationLevelType* pActivationLevelsPerCol,
        std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations) const;

#    ifdef VANILLA_SP_USE_BOOSTING

    // implements _onEvaluateBoostingFromColumnUsage() when local inhib can be computed along x coordinates only
    void _onEvaluateBoostingFromColumnUsageWithLocalInhibAlongX();

    // implements _onEvaluateBoostingFromColumnUsage() when local inhib requires full-blown neighborhood per column
    void _onEvaluateBoostingFromColumnUsageWithFullLocalInhib();

#    endif // VANILLA_SP_USE_BOOSTING

    // implements _onUpdateOverThresholdRatioTarget() when local inhib can be computed along x coordinates only
    void _onUpdateOverThresholdRatioTargetWithLocalInhibAlongX();

    // implements _onUpdateOverThresholdRatioTarget() when local inhib requires full-blown neighborhood per column
    void _onUpdateOverThresholdRatioTargetWithFullLocalInhib();

#  else // hopefully VANILLA_SP_USE_LOCAL_INHIB == VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET

    // implements _getActiveColumnsFromActivationLevels() when bucket inhib mode is selected
    template<typename ActivationLevelType, bool bOutputMinActivation>
    void _getActiveColumnsFromActivationLevelsWithBucketInhib(const ActivationLevelType* pActivationLevelsPerCol,
        std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations) const;

#    ifdef VANILLA_SP_USE_BOOSTING

    // implements _onEvaluateBoostingFromColumnUsage() when bucket inhib mode is selected
    void _onEvaluateBoostingFromColumnUsageWithBucketInhib();

#    endif // VANILLA_SP_USE_BOOSTING

    // implements _onUpdateOverThresholdRatioTarget() when bucket inhib mode is selected
    void _onUpdateOverThresholdRatioTargetWithBucketInhib();

#  endif // value of VANILLA_SP_USE_LOCAL_INHIB 

#endif // VANILLA_SP_USE_LOCAL_INHIB

    // implements _getActiveColumnsFromActivationLevels() when global inhibition is selected
    template<typename ActivationLevelType, bool bOutputMinActivation>
    void _getActiveColumnsFromActivationLevelsWithGlobalInhib(const ActivationLevelType* pActivationLevelsPerCol,
        std::vector<uint16>& vecOutputIndices, uint32* pOutputMinActivations) const;

    // implements _onUpdateOverThresholdRatioTarget() when global inhibition is selected
    void _onUpdateOverThresholdRatioTargetWithGlobalInhib();

    // Will increase synapse permanences to currently active inputs, and decrease synapse permanences to currently inactive
    //   inputs, for each 'active' column.
    void _updateSynapsesOnActiveColumnsTowardsCurrentInput(const uint64* pInputBinaryBitmap,
        const std::vector<uint16>& vecActiveIndices);

    // Will update column usage ratios for all columns, taking into account column activity this round
    void _onEvaluateColumnUsage(const uint16* pRawActivationLevelsPerCol, const uint64* pResultingBinaryBitmap);

    // Will increase all synaptic permanence values on columns which are deemed under-used
    void _onIncreasePermanencesForUnderUsedColums();

    // Will compute the OverThresholdRatio which a column shall strive to reach or exceed, for all columns, based on
    //   neighboring columns usage
    // Note: Disabled: direct call to the various distinct implementations performed on '_compute'
    // void _onUpdateOverThresholdRatioTarget();

    // Temporary buffers for holding bitfields if not provided by caller

    uint64* _pTmpBinaryInputBuffer;
    uint64* _pTmpBinaryOutputBuffer;

    // Other bitfield buffers

    uint64* _pTmpBinaryOverThresholdActivations;
#ifdef VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI
    uint64* _pConnectivityFields;                   // ... here one such bitfield for each minicolumn !
    uint64  _uConnectivityFieldsQwordSizePerColumn;
#endif

    // Other temporary buffers and one-per-column tables

    uint16* _pTmpRawActivationLevelsPerCol;
#ifdef VANILLA_SP_USE_BOOSTING
    uint32* _pTmpBoostedActivationLevelsPerCol;
    uint16* _pBoostingPerCol;
#endif
    float* _pAverageOverThresholdRatioPerColumn;
    float* _pAverageActiveRatioPerColumn;
    float* _pOverThresholdRatioTargetPerColumn;

    // Properties from constructor params

    uint8 _uPotentialConnectivityRadius;
    uint8 _uInhibitionRadius;
    uint8 _uInhibitionSideSize;
    uint8 _uInputSheetsCount;
    float _fOverThresholdTargetVsMaxRatio;
    float _fActivationDensityRatio;

    float _fPotentialConnectivityRatio;
    // additional properties for when bucket inhib mode is selected
    uint8 _uBucketSize;
    uint8 _uBucketCountY;

    uint64 _uColumnUsageIntegrationWindow;

    // Misc.

    uint32* _pTmpTableBest;
    size_t _uCurrentWinnerK;
    uint64 _uEpoch;
    uint64 _uEpochLearning;

    // Last but not least... the list of (proximal) 'Segments'... which are little more than synapse containers.
    Segment* _pSegments;            //  (one per minicolumn)
};

#if defined(VANILLA_SP_SUBNAMESPACE)
    } // namespace VANILLA_SP_SUBNAMESPACE
#endif
} // namespace HTMATCH

