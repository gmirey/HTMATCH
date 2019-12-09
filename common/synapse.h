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
        //  note that its encoding is distinct from a more common '.16b' fixpoint, in that 1.0 corresponds to 65535, not 65536
        k_eSynapticMode_fixed16,

        // Synapse permanence stored on 8b fixed-point.
        //  8b fixPoint is accurate enough to track relevant changes, but is sometimes tight for tweaking learning rates
        //  note that its encoding is distinct from a more common '.8b' fixpoint, in that 1.0 corresponds to 255, not 256
        k_eSynapticMode_fixed8,

        // Synapse permanence stored on 8b fixed-point, as 'k_eSynapticMode_fixed8' above, however distinct from it in that it
        //   uses .16b fixPoint for its learning rate definitions (ie. same as packed modes, see below), so that tweaking
        //   learning rates is made more precise, at the expense of requiring stochastic draws for synaptic updates...
        k_eSynapticMode_fixed8stocha,

        // - - - - - - - - - - - - - - -
        // Packed modes: possibly packed in just 16b (or less?) **together with** a small address (of axonal afferent in
        //   backwards integration mode, or of dendritic segment in full-forward integration mode, for those models supporting
        //   such small addresses => not usable for 'Vanilla' SP or TM in most cases)
        // Hopefully very performance-and-memory-friendly!
        // - - - - - - - - - - - - - - -
        // Rationale:
        // 5b fixPoint (or even 4... or 3?) should be accurate enough to "read-from", even for models which would use those
        //   values as 'weights' to apply to pre-synaptic potentials (not the case of HTM, anyway). However, wanting to apply
        //   small incremental changes of anything but 'coarse' deltas of 1/32nd of the available range in the 5b case (between
        //   a fainting synapse and the strongest connected) would require the help of stochastic methods. eg, 25% chance of
        //   applying a +1/32nd permanence increase would, on the long term, be equivalent with applying a +1/128th increase...
        // This could however impact performance of those synaptic changes (requires drawing from a RNG)...
        //   ...but other than that, should do fine!
        // Note that learning rates for all packed modes will be stored as .16b fixPoint.
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

        /*
        
        // TODO: An idea to explore in the future... or not (see getDefaultCrystalizedThreshold())

        // - - - - - - - - - - - - - - -
        // Custom modes: either plain or packed fixpoint modes, where we use a custom (and quite ad-hoc) encoding scheme,
        //   where there is room for 'flagging' a synapse as having been strongly connected at one point in its lifetime.
        // When flagged as such, it is possible to forbid this synapse from ever fainting away completely, even when dynamically
        //   set-back by recent input distributions...
        // Rationale: this could reflect some biologically plausible explanation for why we could unlearn to ride a bike when
        //   training with inverted controls... but the re-learn phase would be somewhat faster than learning it from scratch.
        // Alternatively, we could maybe imagine a small (stochastic?) percentage of synapses allowed to reach the very-top
        //   of permanence values, and then never allow any of them to be set-back ... ? This may work since if those top-count
        //   are below what is the required threshod to detect a coincidence of inputs, then we've in effect "unlearned" to
        //   ride... but less synapses would be required to train for the remaining count up to threshold again?
        //   ...see getDefaultCrystalizedThreshold() in 'SynapticDefaultsHelper' for a first-step towards this alternative idea:
        //     synapses over the crystalized threshold could be harder and harder to train (an effect which is often taken into
        //     account by other synaptic-'weight' schemes with techniques of diminishing returns, but not by canonic HTM). What
        //     we could allow here, however, is to make this 'harder' setting really **hard**, AND also make it harder for them
        //     to *untrain*... perhaps even all the way (depending on config options) to outright-impossible to untrain when
        //     having reached top-perm => in effect maybe achieving the specification discussed above.
        // - - - - - - - - - - - - - - -

        // 16b fixpoint custom mode.
        k_eSynapticMode_custom16,
        
        // 8b fixpoint custom mode.
        //   0..111 are repeated twice (flagged or unflagged) => 0..223 ; then 224..255 for strongly connected (=> flagged since)
        k_eSynapticMode_custom8,

        // 5b packed custom mode.
        //   0..13 are repeated twice (flagged or unflagged) => 0..27 ; then 28..31 for strongly connected (=> flagged since)
        k_eSynapticMode_custom5,

        // 4b packed custom mode.
        //   0..6 are repeated twice (flagged or unflagged) => 0..13 ; then 14 or 15 are strongly connected (=> flagged since)
        k_eSynapticMode_custom4,

        */
    };

    // - - - - - - - - - - - - - - -
    // A little bit of template meta-programming here, shall we?
    // Sorry, guys... Either that, or previous version C-style macros...
    // - - - - - - - - - - - - - - -

    // Static-Helper 'class' for types and consts, given a value of the 'eSynapticMode' enum
    //   all valid specializations shall typedef the following:
    //      SynPermanence_t: the type of a stored synapse permanence value
    //      SynPermSigned_t: the (req. signed) type of intermediate values used when doing synaptic update computations
    //   also, all valid specializations shall define the following static const bool:
    //      IS_FLOAT:  whether the synapse permanence is stored with floating point (true) or fixPoint (false)
    //      IS_PACKED: whether the synapse permanence is meant to be packed with the address (true) or in another storage (false)
    //      IS_RATE16: whether the learning rates are expressed in .16b fixPoint (true) or in same type as perm (false)
    //   also the MaxVal() constexpr returning, as signed perm type, the value corresponding to floating-point '1.0' permanence
    //   and the EpsVal() constexpr returning, for those types where IS_RATE16 is true, the .16b fixPoint value of a +1 epsilon.
    // See the explicit specializations below...
    template<eSynapticMode T_SYNAPTIC_MODE>
    struct SynapseKind {};

    // - - - - - - - - - - - - - - -
    // explicit specializations for the 'SynapseKind' templatized class

    template<> struct SynapseKind<k_eSynapticMode_float32> {
        typedef float SynPermanence_t;  typedef float SynPermSigned_t;
        static const bool IS_FLOAT = true;
        static const bool IS_PACKED = false;
        static const bool IS_RATE16 = false;
        static constexpr SynPermSigned_t MaxVal() { return 1.0; }
        static constexpr uint16 EpsVal() { return 0; } // unused here, but required still defined
    };
    template<> struct SynapseKind<k_eSynapticMode_fixed16> {
        typedef uint16 SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = false;
        static const bool IS_RATE16 = false;
        static constexpr SynPermSigned_t MaxVal() { return 65535; }
        static constexpr uint16 EpsVal() { return 0;    } // unused here, but required still defined
    };
    template<> struct SynapseKind<k_eSynapticMode_fixed8>  {
        typedef uint8 SynPermanence_t;  typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = false;
        static const bool IS_RATE16 = false;
        static constexpr SynPermSigned_t MaxVal() { return 255; }
        static constexpr uint16 EpsVal() { return 0; } // unused here, but required still defined
    };
    template<> struct SynapseKind<k_eSynapticMode_fixed8stocha>  {
        typedef uint8 SynPermanence_t;  typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = false;
        static const bool IS_RATE16 = true;
        static constexpr SynPermSigned_t MaxVal() { return 255; }
        static constexpr uint16 EpsVal() { return 257; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed5> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static const bool IS_RATE16 = true;
        static constexpr SynPermSigned_t MaxVal() { return 31; }
        static constexpr uint16 EpsVal() { return 2048; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed4> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static const bool IS_RATE16 = true;
        static constexpr SynPermSigned_t MaxVal() { return 15; }
        static constexpr uint16 EpsVal() { return 4096; }
    };
    template<> struct SynapseKind<k_eSynapticMode_packed3> {
        typedef u8fast SynPermanence_t; typedef i32fast SynPermSigned_t;
        static const bool IS_FLOAT = false;
        static const bool IS_PACKED = true;
        static const bool IS_RATE16 = true;
        static constexpr SynPermSigned_t MaxVal() { return 7; }
        static constexpr uint16 EpsVal() { return 8192; }
    };

    // Static-Helper 'class' for default-values regarding persistence and learning rates,
    //   given a value of the 'eSynapticMode' enum.
    // all valid specializations shall define the following static constexpr:
    //   getDefaultInactiveDec() : the default decrease to apply to synapses of inactive inputs to active columns
    //     when training an HTM-like SP. (shall return SynPermSigned_t, and shall depend on the 'IS_RATE16' flag)
    //   getDefaultActiveInc() : the default increase to apply to synapses of inactive inputs to active columns
    //     when training an HTM-like SP. (shall return SynPermSigned_t, and shall depend on the 'IS_RATE16' flag)
    //   getDefaultBelowStimInc() : the default increase to apply to all synapses of underperforming columns
    //     when training an HTM-like SP. (shall return SynPermSigned_t, and shall depend on the 'IS_RATE16' flag)
    //   getDefaultConnectionThreshold() : the default threshold at or above which we consider a synapse as connected in an
    //     HTM-like binary-weight mode. (shall return SynPermanence_t)
    //   getDefaultCrystalizedThreshold() : the default threshold at or above which we may apply various techniques of
    //     "harder-to-train" (or untrain?) effects to synapses, in models configured for them. (shall return SynPermanence_t)
    //   getDefaultSpreadAtInit() : the default spread, below or above the connection threshold, we wish to use when
    //     randomly drawing the permanence value of each synapse during initialization phase. (shall return SynPermSigned_t,
    //     and shall be .16b fixPoint whenever non-float)
    //   getDefaultSpreadAtReset() : the default spread, below or above the connection threshold, we wish to use when
    //     randomly drawing the permanence value of a synapse which has been switched to another cell, in models configured
    //     for using such techniques. (shall return SynPermSigned_t, and shall be .16b fixPoint whenever non-float)
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
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 0.5019607843f; }                 //           (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 1.0; }
        static constexpr typename SynapseKind<k_eSynapticMode_float32>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 1.0; }
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_fixed16> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257; }     // div 65535 => 0.00392156863      (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // div 65535 => 0.0235294118       (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // div 65535 => 0.0274509804       (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 16448u; }  // div 65535 => 0.2509803922       (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 32896u; }  // div 65535 => 0.5019607843       (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed16>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_fixed8> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermSigned_t
            getDefaultInactiveDec()           { return 1; }       // div 255 => 0.00392156863
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermSigned_t
            getDefaultActiveInc()             { return 6; }       // div 255 => 0.0235294118
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 7; }       // div 255 => 0.0274509804
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 64u; }     // div 255 => 0.2509803922
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 128u; }    // div 255 => 0.5019607843
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65535; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65535; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_fixed8stocha> {
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermSigned_t
            getDefaultInactiveDec()           { return 1; }       // .16b => 0.00392150879      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermSigned_t
            getDefaultActiveInc()             { return 6; }       // .16b => 0.0235290527       (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 7; }       // .16b => 0.0274505615       (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 64u; }     // div 255 => 0.2509803922    (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 128u; }    // div 255 => 0.5019607843    (same as 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65535; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_fixed8stocha>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65535; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed5> {
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b (= 0.00392150879)     (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b (= 0.0235290527)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b (= 0.0274505615)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 8u; }      // .5b* => 0.25               (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 16u; }     // .5b* => 0.5                (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_packed5>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed4> {
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b (= 0.00392150879)     (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b (= 0.0235290527)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b (= 0.0274505615)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 4u; }      // .4b* => 0.25               (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 8u; }      // .4b* => 0.5                (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_packed4>::SynPermSigned_t
            getDefaultSpreadAtReset()         { return 65536; }   // .16b ratio (= 1.0)
    };
    template<> struct SynapticDefaultsHelper<k_eSynapticMode_packed3> {
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermSigned_t
            getDefaultInactiveDec()           { return 257;  }    // .16b (= 0.00392150879)     (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermSigned_t
            getDefaultActiveInc()             { return 1542; }    // .16b (= 0.0235290527)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermSigned_t
            getDefaultBelowStimInc()          { return 1799; }    // .16b (= 0.0274505615)      (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermanence_t
            getDefaultConnectionThreshold()   { return 2u; }      // .3b* => 0.25               (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermanence_t
            getDefaultCrystalizedThreshold()  { return 4u; }      // .3b* => 0.5                (differs slightly from 8b)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermSigned_t
            getDefaultSpreadAtInit()          { return 65536; }   // .16b ratio (= 1.0)
        static constexpr typename SynapseKind<k_eSynapticMode_packed3>::SynPermSigned_t
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
                SynPermSigned_t binaryConnectedDraw = SynPermSigned_t(rng.getNext() & 1u); // 0 or 1
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
            if (SynapseKind<T_SYNAPTIC_MODE>::IS_RATE16)
                return value >= SynPermSigned_t(0) && value < SynPermSigned_t(32768);  // rate16 deltas are .16b fixpt
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

