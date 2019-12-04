/* -----------------------------------
 * HTMATCH
 * synapse.h
 * -----------------------------------
 * Defines types and tools for dealing with different synaptic configuration
 *   choices regarding the synaptic footprints, for encoding the 'permanence'
 *   value of synapses, in particular.
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
#ifndef _HTMATCH_SYNAPSE_H
#define _HTMATCH_SYNAPSE_H

#include "tools/system.h"
#include "tools/rand.h"
#include <algorithm>

namespace HTMATCH {

    // Different synaptic modes (influencing footprint of each synapse) one can chose from,
    //   for using a specific implementations of some models proposed by this project
    enum eSynapticMode {

        // - - - - - - - - - - - - - - -
        // Floating-point modes
        // - - - - - - - - - - - - - - -

        // Note that we don't even bother proposing a 64b 'double' configuration here... now c'mon!

        // Synapse permanence stored on 32b float (standard-HTM style)
        k_eSynapticMode_float32,

        // - - - - - - - - - - - - - - -
        // Fixpoint modes
        // - - - - - - - - - - - - - - -

        // Synapse permanence stored on 16b fixed-point.
        //  16b fixPoint is more than enough accuracy to represent virtually any user-chosen learning rate, and is thus almost
        //  always preferrable to float32b for large models (where synaptic weights would already dominate every other data)
        k_eSynapticMode_fixed16,

        // Synapse permanence stored on 8b fixed-point.
        //  8b fixPoint is accurate enough to track relevant changes, but is sometimes tight for tweaking learning rates
        k_eSynapticMode_fixed8,

        // - - - - - - - - - - - - - - -
        // Packed modes: possibly packed in just 16b (or less?) **together with** a small address of axonal afferent,
        //   (for those models supporting such small addresses => not usable for 'Vanilla' SP or TM in most cases)
        // Hopefully very performance-and-memory-friendly!
        // - - - - - - - - - - - - - - -
        // Rationale:
        // 5b fixPoint (or even 4... or 3?) should be accurate enough to "read-from", even for models which would use those
        //   values as 'weights' to apply to pre-synaptic potentials (not the case of HTM, anyway). However, wanting to apply
        //   small incremental changes of anything but 'coarse' deltas of 1/32nd of the available range in the 5b case (between
        //   a fainting synapse and the strongest connected) would require the help of stochastic methods. eg, 25% chance of
        //   applying a +1/32nd permanence increase would, on the long term, be equivalent with applying a +1/128th increase...
        // This could however impact performance of those synaptic changes (requires drawing from a RNG), and make tweaking
        //   learning rates more complicated... but other than that, should do fine!
        // - - - - - - - - - - - - - - -

        // Synapse permanence stored on 5b fixed-point
        //   packable in just 16b together with an 11b address of axonal afferent
        k_eSynapticMode_packed5,

        // Synapse permanence stored on 4b fixed-point
        //   packable in just 16b together with a 12b address of axonal afferent
        //   packable in just 12b together with a 8b address of axonal afferent
        k_eSynapticMode_packed4,

        // Synapse permanence stored on 3b fixed-point
        //   packable in just 16b together with a 13b address of axonal afferent
        //   packable in just 12b together with a 9b address of axonal afferent
        k_eSynapticMode_packed3,
    };

    // - - - - - - - - - - - - - - -
    // A little bit of template meta-programming here, shall we?
    // Sorry, guys... Either that, or previous version C-style macros...
    // - - - - - - - - - - - - - - -

    // Static-Helper 'class' for types and consts, given a value of the 'eSynapticMode' enum
    // See the explicit specializations below...
    template<eSynapticMode T_SYNAPTIC_MODE>
    struct SynapseKind {};

    // - - - - - - - - - - - - - - -
    // explicit specializations for the 'SynapseKind' templatized class

    template<> struct SynapseKind<k_eSynapticMode_float32> {
        typedef float SynPermanence_t;  typedef float SynPermSigned_t;
        static const bool IS_FLOAT = true;
        static const bool IS_PACKED = false;
        static constexpr SynPermSigned_t MaxVal() { return 1.0; }
    };
    template<> struct SynapseKind<k_eSynapticMode_fixed16> {
        typedef uint16 SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = false;
        static constexpr SynPermSigned_t MaxVal() { return 65535; }
    };
    template<> struct SynapseKind<k_eSynapticMode_fixed8>  {
        typedef uint8 SynPermanence_t;  typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = false;
        static constexpr SynPermSigned_t MaxVal() { return 255; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed5> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static constexpr SynPermSigned_t MaxVal() { return 31; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed4> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static constexpr SynPermSigned_t MaxVal() { return 15; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed3> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static constexpr SynPermSigned_t MaxVal() { return 7; }
    };

    // Static-Helper 'class' for default-values regarding persistence and learning rates,
    //   given a value of the 'eSynapticMode' enum.
    // See the explicit specializations below...
    template<eSynapticMode T_SYNAPTIC_MODE>
    struct SynapticDefaultsHelper {};

    // - - - - - - - - - - - - - - -
    // explicit specializations for the 'SynapticDefaultsHelper' templatized class

    template<> struct SynapticDefaultsHelper<k_eSynapticMode_float32> {
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultInactiveDec()           { return 0.00392156863f; }                //           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultActiveInc()             { return 0.0235294118f; }                 //           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 0.0274509804f; }                 //           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 0.2509803922f; }                 //           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 1.0; }
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 1.0; }
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_fixed16> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257; }     // .16b' => 0.00392156863          (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b' => 0.0235294118           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b' => 0.0274509804           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 16448u; }  // .16b' => 0.2509803922           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_fixed8> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 1; }       // .8b' => 0.00392156863
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 6; }       // .8b' => 0.0235294118
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 7; }       // .8b' => 0.0274509804
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 64u; }     // .8b' => 0.2509803922
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65535; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65535; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed5> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b ratio (= 0.00392150879)    (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b ratio (= 0.0235290527)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b ratio (= 0.0274505615)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 8u; }      // .5b* => 0.25                    (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed4> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b ratio (= 0.00392150879)    (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b ratio (= 0.0235290527)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b ratio (= 0.0274505615)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 4u; }      // .4b* => 0.25                    (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed3> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b ratio (= 0.00392150879)    (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b ratio (= 0.0235290527)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b ratio (= 0.0274505615)     (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 2u; }      // .3b* => 0.25                    (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };

    // Static-Helper 'class' for everything which does not require explicit template specialization any more at this point,
    //   (at least if we accept a little bit of superfluous casts, and if we trust the compiler to optimize-away some
    //    'if (SynapseKind<T_SYNAPTIC_MODE>::IS_PACKED)' or 'if (SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT)' conditionnals...)
    // In particular, provides handy methods for initializing new synapses, and validity checks for configuration parameters.
    template<eSynapticMode T_SYNAPTIC_MODE>
    struct SynapticConfHelper {
        typedef typename SynapseKind<T_SYNAPTIC_MODE>::SynPermanence_t      SynPermanence_t;
        typedef typename SynapseKind<T_SYNAPTIC_MODE>::SynPermSigned_t      SynPermSigned_t;

        template<bool bAllowUnconnected, bool bAllowConnected>
        static FORCE_INLINE SynPermanence_t drawNewPermanence(Rand& rng, SynPermSigned_t thresholdAsSigned,
            SynPermSigned_t rangeBelow, SynPermSigned_t rangeAbove) FORCE_INLINE_END
        {
            if (bAllowUnconnected && bAllowConnected) {
                // ... choose whether it should be an initially connected or unconnected synapse (a fixed 50% chance of each)
                SynPermSigned_t binaryConnectedDraw = SynPermSigned_t(pSynRand->getNext() & 1u); // 0 or 1
                // impl unbranching choice by multiplying both result with either binaryConnectedDraw or its binary negation:
                SynPermSigned_t whenConnected = _drawConnected(rng, thresholdAsSigned, rangeAbove);
                SynPermSigned_t whenUnconnected = _drawUnconnected(rng, thresholdAsSigned, rangeBelow);
                return SynPermanence_t(whenConnected * binaryConnectedDraw +
                                       whenUnconnected * (SynPermSigned_t(1) - binaryConnectedDraw));
            } else if (bAllowUnconnected) {
                return SynPermanence_t(_drawUnconnected(rng, thresholdAsSigned, rangeBelow));
            } else if (bAllowConnected) {
                return SynPermanence_t(_drawConnected(rng, thresholdAsSigned, rangeAbove));
            } else { // dumb case, really...
                return SynPermanence_t(thresholdAsSigned); // not less dumb, but... oh, well...
            }
        }

        static void getSpreadRanges(SynPermSigned_t thresholdAsSigned, SynPermSigned_t spreadFactor, bool isSpreadEven,
            SynPermSigned_t* pOutRangeAbove, SynPermSigned_t* pOutRangeBelow)
        {
            SynPermSigned_t fullRangeAbove = SynapseKind<T_SYNAPTIC_MODE>::MaxVal() - thresholdAsSigned;
            SynPermSigned_t fullRangeBelow = thresholdAsSigned;
            if (!SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT) {
                fullRangeAbove += SynPermSigned_t(1);   // will draw to range value excluded => 1 more to get to maxval
                if (!SynapseKind<T_SYNAPTIC_MODE>::IS_PACKED) { // packed modes allow '0' for encoding non-null, but otherwise:
                    fullRangeBelow -= SynPermSigned_t(1);   // 1 less to avoid 0, since '_drawUnconnected' will add -1
                }
            }
            if (isSpreadEven) {
                // the following is quite plainly what the 'isSpreadEven' flag means. And yes, we're levelling those guys down...
                SynPermSigned_t levelledRange = std::min(fullRangeAbove, fullRangeBelow);
                *pOutRangeAbove = *pOutRangeBelow = _getRangeFromSpread(spreadFactor, levelledRange);
            } else {
                *pOutRangeAbove = _getRangeFromSpread(spreadFactor, fullRangeAbove);
                *pOutRangeBelow = _getRangeFromSpread(spreadFactor, fullRangeBelow);
            }
        }

        static bool isValidDelta(SynPermSigned_t value) {
            // delta values shall be non-negative and less than 0.5 (on normalized [0.0 .. 1.0] range)
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_PACKED)
                return value >= SynPermSigned_t(0) && value < SynPermSigned_t(32768);  // packed deltas are .16b fixpt
            else
                return value >= SynPermSigned_t(0) && 
                       value < (SynapseKind<T_SYNAPTIC_MODE>::MaxVal() / SynPermSigned_t(2));
        }
        static bool isValidThreshold(SynPermanence_t value) {
            // connection threshold shall be strictly above min and strictly below max
            return (SynapseKind<T_SYNAPTIC_MODE>::IS_PACKED ?
                        SynPermSigned_t(value) >= SynPermSigned_t(0) :  // packed modes also allow '0' for encoding non-null
                        SynPermSigned_t(value) > SynPermSigned_t(0)) &&
                   SynPermSigned_t(value) < SynapseKind<T_SYNAPTIC_MODE>::MaxVal();
        }
        static bool isValidSpread(SynPermSigned_t value) {
            // spread shall be [0.0 .. 1.0]
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT)
                return value >= SynPermSigned_t(0.0f) && value <= SynPermSigned_t(1.0f);
            else
                return value >= SynPermSigned_t(0) && value <= SynPermSigned_t(65536); // non-float spreads are .16b fixpt
        }

    private:

        static FORCE_INLINE SynPermSigned_t _drawUnconnected(Rand& rng, SynPermSigned_t thresholdAsSigned,
            SynPermSigned_t rangeBelow) FORCE_INLINE_END
        {
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT) {
                return thresholdAsSigned - SynPermSigned_t(rng.getNextAsFloat01()) * rangeBelow;
            } else {
                return thresholdAsSigned - SynPermSigned_t(1) - SynPermSigned_t(rng.drawNextFromZeroToExcl(rangeBelow));
            }
        }

        static FORCE_INLINE SynPermSigned_t _drawConnected(Rand& rng, SynPermSigned_t thresholdAsSigned,
            SynPermSigned_t rangeAbove) FORCE_INLINE_END
        {
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT) {
                return thresholdAsSigned + SynPermSigned_t(rng.getNextAsFloat01()) * rangeAbove;
            } else {
                return thresholdAsSigned + SynPermSigned_t(rng.drawNextFromZeroToExcl(rangeAbove));
            }
        }

        static SynPermSigned_t _getRangeFromSpread(SynPermSigned_t spreadFactor, SynPermSigned_t maxRange) {
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_FLOAT) {
                return spreadFactor * maxRange;
            } else { // 'spreadFactor' expressed .16b for non-floating point modes
                // we switch to unsigned here to avoid cases where mul would set bit 31 then interpret it as a sign bit
                //   (fine since it is expected that all input values are known non-negative)
                return SynPermSigned_t((uint32(spreadFactor) * uint32(maxRange)) >> 16u);
            }
        }

    };

} // namespace HTMATCH 

#endif // _HTMATCH_SYNAPSE_H

