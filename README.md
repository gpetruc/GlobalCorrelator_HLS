# Correlator common repository

Algorithms for Correlator Layer 1 and Layer 2

## Structure of this repository:

General directories
 * `dataformats`: header files defining the object formats and some other constants (e.g. layer 1 object multiplicites)
 * `data`: data files dumped from CMSSW simulation, for testing
 * `utils`: algorithm-independent utilities, e.g. to dump pattern files from vectors of `ap_uint` or to load data files 

Algorithm directories:
 * `pf`: PF algo
 * `puppi`: Linearized Puppi algo, both within tracker coverage and outside
 * `l1-regionizer/tdr`: Layer 1 regionizer from the TDR, used in the Barrel 
 * `l1-regionizer/multififo` : Layer 1 regionizer with a matrix of FIFOs, used in the HGCal
 * `l1-converters` code for the conversions of layer 1 input objects to the format used by PF


## Setup of the code

Within each algorithm directory, there should be a readme file, and HLS code for synthesis should be in a `firmware` directory. 

Some compile-time constants are defined to defining the detector region (for layer 1), and the setup used:
 * The detector region is set with `-DREG_(region)` with region being one of `Barrel`, `HGCal`, `HGCalNoTK`, `HF`. This drives the configuration of the object multiplicities (set in `firmware/data.h` and potentially further customized depending on the test board used), and the parameters of the algorithms (set in the corresponding header files; currently this is the case only for puppi, not for PF).
 * The board used is set with `-DBOARD_(board)`, which drives the code used to serialize the inputs and outputs. A generic setup in which the algorithms are run without any wrapper is defined with `-DBOARD_none`

## Implementation status

* PF compiles, runs and synthethizes, both in the barrel (pfalgo3) and hgcal (pfalgo2hgc). An example wrapping is tested for `BOARD_VCU118`, where inputs and outputs are serialized as 72 bit objects (and dumped as two 64 bit words in the pattern file)
* Puppi compiles, runs and synthethizes in all detector regions. In the wrapped version, inputs are 72-bit wide while outputs are 64-bit wide.
* Dump files now contain new emulator classes that inherit from the firmware ones, replacing the DiscretePFInputs.
* PF & Puppi emulators have been converted to C++ classes more similar to the CMSSW emulators
* Everything else is not tested and most likely won't even compile yet

## Pending items

* Finish cleaning up C++ interface of PF & Puppi emulators, e.g. take configurations as floats. Eventually replace the CMSSW algo.
* Improve Puppi implementation: use `ap_fixed` instead of bitshifts by hand, and use 2D LUT instead of the current ugly LUT-generating macro.
* Introduce the fiducial cut and shift to global coordinates in Puppi
* Introduce header files for the inputs and the decoders
* Possibly switch to a packing & unpacking implementation with varadic templates to make the code more compact
* See if the code for the lookup of the ptErr in the PF can be improved.
* Get the rest of the code compiling and synthetizing
* Introduce scripts for testing code before integration
* Sync code with CMSSW, run code-checks & code-format
