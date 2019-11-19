/* -----------------------------------
 * HTMATCH
 * VanillaHTMConfig.h
 * -----------------------------------
 * Defines the default dimensions for the vanilla-HTM-like Spatial Pooler and
 *   Temporal Memory, and all its available configuration options.
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
#ifndef _VANILLA_HTM_CONFIG_H
#define _VANILLA_HTM_CONFIG_H

//----------------------------------------
// In this implementation, our cortical sheets are all statically set to 64 x 32 "mini"columns
//  => 2048 minicolumns, which would cover about 3.28 mm² of cortical surface on average, handily representing a "macrocolumn"
//  (one minicolumn is thought to cover 40µm x 40µm)
//----------------------------------------
 
#define VANILLA_HTM_SHEET_WIDTH         64u                 // so, 64 x
#define VANILLA_HTM_SHEET_HEIGHT        32u                 // ... 32, right ?

// And then a few values derived from those... We'd rather have them explicitely shown here rather than using macro formulas...
//   although it means one would need to update them by hand if we were to change width or height. Then again, changing those
//   without giving a good look to every part of the code where those dimensions are sometimes just assumed (such as, when 
//   choosing the types (and bit footprint) of values used to compute stuff)... would perhaps not be a very wise move.
//   in any case, avoid setting height greater than width (though equal should do fine), or depart from powers of two.

#define VANILLA_HTM_SHEET_2DSIZE        2048u               // width * height
#define VANILLA_HTM_SHEET_BYTES_BINARY  256u                // 2D size / 8 (number of bytes in the activation bitfield of a sheet)

#define VANILLA_HTM_SHEET_SHIFT_DIVX    6u                  // 64 is 2^6
#define VANILLA_HTM_SHEET_SHIFT_DIVY    5u                  // 32 is 2^5
#define VANILLA_HTM_SHEET_XMASK         0x003Fu             // 6 lsb set (masking remainder of div 64)
#define VANILLA_HTM_SHEET_YMASK         0x001Fu             // 5 lsb set (masking remainder of div 32)

#define VANILLA_HTM_SHEET_SHIFT_DIV2D   11u                 // 2048 is 2^11
#define VANILLA_HTM_SHEET_2DMASK        0x07FFu             // 11 lsb set (masking remainder of div 2048)

#define VANILLA_HTM_SHEET_HALFWIDTH     32u                 // 64/2
#define VANILLA_HTM_SHEET_HALFHEIGHT    16u                 // 32/2
#define VANILLA_HTM_SHEET_XHALFMASK     0x001Fu             // 5 lsb set (masking remainder of div 32)
#define VANILLA_HTM_SHEET_YHALFMASK     0x000Fu             // 4 lsb set (masking remainder of div 16)

// 2048 minicolumns x 32 "depths" is 65536 positions (=> max addresses on 16b)
#define VANILLA_HTM_SHEET_MAX_DEPTH     32u              


//----------------------------------------
// Vanilla SpatialPooler behavior configurations
//   #define VANILLA_SP_CONFIG to one of those values before including "VanillaSPGen.h"
//   Will default to 1 if undefined
//----------------------------------------

// - - - - - - - - - - - - - - - - - - - -
// Configurations without boosting

#define VANILLA_SP_CONFIG_CONST_GLOBAL_NOBOOSTING     1    // global inhibition, no boosting, connectivity field opti
#define VANILLA_SP_CONFIG_CONST_BUCKET_NOBOOSTING     2    // bucket inhibition, no boosting, connectivity field opti
#define VANILLA_SP_CONFIG_CONST_LOCAL_NOBOOSTING      3    // local inhibition, no boosting, connectivity field opti

// - - - - - - - - - - - - - - - - - - - -
// Configurations with boosting at the 0.1 "sweetspot" as reported by @marty1885
// @see https://discourse.numenta.org/t/mapping-the-hyper-parameter-space-of-classifcation-using-sp/6815/4
// also note that our boosting values here will be implemented as a simple linear interpolant, following this discussion

// ... Either without changing vanilla philosophy wrt. the 'updateRadius' concern

#define VANILLA_SP_CONFIG_CONST_GLOBAL                4    // global inhibition, boosting formula at 0.1, connectivity field opti
#define VANILLA_SP_CONFIG_CONST_BUCKET                5    // bucket inhibition, boosting formula at 0.1, connectivity field opti
#define VANILLA_SP_CONFIG_CONST_LOCAL                 6    // local inhibition, boosting formula at 0.1, connectivity field opti

// ... Or to the contrary fixing vanilla 'updateRadius' concept in various ways

#define VANILLA_SP_CONFIG_CONST_LOCAL_CORRECTEDUPDATERAD      7    // local inhibition, having fixed the span avg computation
#define VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD             8    // local inhibition, keeping inhib radius constant
#define VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_ALL_OPTIM   9    // local inhibition, constant radius, optim neighbor-sampling
#define VANILLA_SP_CONFIG_CONST_LOCAL_NOUPDATERAD_GAUSSTEST   10   // local inhibition, gaussian filter over radius +
                                                                   //   single-winner over 7x7 kernel (=> fixed ~2% sparsity)

//----------------------------------------
// Vanilla SpatialPooler synaptic configurations
//   #define VANILLA_SP_SYNAPSE_KIND to one of the following values before including "VanillaSPGen.h"
//   Will default to 1 if undefined
//----------------------------------------
 
#define VANILLA_SP_SYNAPSE_KIND_CONST_USE_FLOAT32     1    // synapse permanence is [0.0f .. 1.0f] stored in 32b floating point
#define VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED16     2    // synapse permanence is [0 .. 65535] stored in uint16,
                                                           //   where max value of 65535 represents 1.0
#define VANILLA_SP_SYNAPSE_KIND_CONST_USE_FIXED8      3    // synapse permanence is [0 .. 255] stored in uint8,
                                                           //   where max value of 255 represents 1.0

//----------------------------------------
// Vanilla SpatialPooler, other configuration constants
//----------------------------------------

// Some limits for vanilla SP

#define VANILLA_SP_MAX_WINNERS                   256u      // Enough to consider up to 8% SP density even with global inhib on.
#define VANILLA_SP_MAX_SYNAPSES_PER_SEG          4096u     // Enough for each column able to connect to up to 50% pre-synaptic
                                                           //   cells from a full, 4-deep input-sheet

// Some defaults for vanilla SP 

#define VANILLA_SP_DEFAULT_POTENTIAL_RADIUS      12u       // Potential radius => in a square 25x25 (not used for depth)
#define VANILLA_SP_DEFAULT_POTENTIAL_RATIO       0.5f      // Potential ratio  => here 50% of presyn-cell in pot area get a syn
#define VANILLA_SP_DEFAULT_DENSITY_RATIO         0.02f     // The resulting SP activation shall be 2% dense (=> about 41 columns)
#define VANILLA_SP_DEFAULT_STIMULUS_THRESHOLD    3u        // Disallow columns with less than 3 active presyn cells to fire at all
#define VANILLA_SP_DEFAULT_TARGET_VS_MAX_RATIO   0.5f      // Columns will strive to be over threshold at least 50% of the time
                                                           //   relative to max of neighbors in inhib radius
#define VANILLA_SP_OVERTHRESHOLD_INIT            0.5f      // Initial value of OverThreholdRatio on each column of a new SP
#define VANILLA_SP_DEFAULT_INTEGRATION_WINDOW    1000uLL   // Integration window for moving averages of column activity
#define VANILLA_SP_DEFAULT_SEED                  0u        // No seed override by default (will have default seed of 'Rand')

// Miscellaneous

#define VANILLA_SP_BOOSTING_VALUE_SWEET                 0.1f // cf. results from @marty1885

#define VANILLA_SP_LOCAL_INHIB_TYPE_NOMINAL             1    // local inhibition is centered on currently considered column
#define VANILLA_SP_LOCAL_INHIB_TYPE_BUCKET              2    // local inhibition is in fact setup as fixed, separated "buckets"

#define VANILLA_SP_UPDATERAD_KIND_CONST_CORRECTED       1    // Correcting span avg computation towards assumed original intent
#define VANILLA_SP_UPDATERAD_KIND_CONST_NOUPDATE        2    // Just... never update radius.
                                                             // Period.
                                                             // I mean it.

#define VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_ALGORITHM   1    // multi-pass optimizations and tweaks applied to neighbor searches.
#define VANILLA_SP_NEIGHBORHOOD_OPTIM_CONST_GAUSSTEST   2    // diff against med-sized gaussian filter, then one-winner on 7x7
                                                             //   kernel (=>fixed 2% density)

#define VANILLA_SP_MIN_AREA_SIDE_SIZE                   6u   // less than radius 3 (inhib or potential) and we'd be better
                                                             //   resorting to global

#define VANILLA_SP_SCALE_THRESHOLD_WHEN_BOOSTED         1    // Scale stimulus threshold with boosting when we select k-winners

// Using the following optimization was found always better at this point
//   if defined, each connected synapse is set as a '1' in a bitfield associated to all presynaptic cells,
//   then we can perform a brute-force bitwise 'AND' against an input bitfield to compute active count for a segment.
#define VANILLA_SP_USE_CONNECTIVITY_FIELD_OPTI                1

#endif // _VANILLA_HTM_CONFIG_H

