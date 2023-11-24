# Introduction

Example of an algorithm to compute MET implemented using lookup tables for complex mathematical functions (sin, cos, sqrt).

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vivado HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `data.h`: define the dataformats
   * `algo.h`, `algo.cpp`: header and source file for the code to be synthethised
* `algo_ref.cpp`: a clean and readable reference implementation of the algorithm, for bitwise validation, and floating point implementation
* `algo_test.cpp`: C++ testbench, that compares the implementation to be synthetised to the reference one to make sure they're bitwise identical (i.e. that any re-writing of the C++ code to make HLS work better didn't change the results), and to the floating point version (to check the accuracy of the discretized version)

# Running the example
## Batch mode using Tcl script
`vitis_hls -f run_hls.tcl`

## To open the project in the GUI
After creating the project using the tcl script (default name from script is `proj`)

`vitis_hls -p proj`

## Implementation with LUT

The default implementation, with Vivado 2023.2, yields Latency 10, Resources: 10 BRAM, 14 DSP, 1686 FF, 1126 LUT

We can then optimize further, packing sin and cos into a single LUT (see the commented code), which reduces the block RAMs to 7 with no extra cost. 
