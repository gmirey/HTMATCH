# HTMATCH
HTM-Able Topological Cortical Hunks
-

HTM is a theory of neocortical function developped by Numenta with an open research philosophy.
You can read about it on https://numenta.com/ or participate in the active community at https://discourse.numenta.org/

HTMATCH aims to be a C++ architecture for simulating cortical areas, taking advantage of biologically plausible topologies, and typical range at which cells can connect to their neighbors, to in fact *increase* performance.

It is a strong design intent to keept the final framework model-agnostic, but allow it to run models abiding to HTM philosophies in particular (hence the 'HTM-Able' designation).

Current proposal of optimized, but "vanilla" HTM implementations is but a first step.

Already available:
-

- Optimized version of HTM Spatial Pooler
* * should work best when compiled for x64 targets, with either MSVC or GCC
* * perf reports to be found at https://discourse.numenta.org/t/topologies-in-the-brain-and-how-to-model-them/6783/15
* * code is a little heavy on preprocessor macros at this point, having devised this scheme allowing to select a particular version and run tests against it. Tried my best to make this hopefully manageable enough.

Soon to be available:
-

- Further optimized versions of HTM Spatial Pooler (yet faster local inhibition)

- Implementation of HTM Temporal Memory.

To come at some point:
-

- Topological architecture intended to support multi-area computational models
