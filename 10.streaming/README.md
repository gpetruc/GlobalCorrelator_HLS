# Introduction

This is an attempt to make in HLS a module that processes a stream of events with some internal state, using the `hls_stream` library.

The intended behaviour is the following: a new event comes every N clock cycles. For each event, the module reads one threshold (A), given at the start, and a set of N numbers B(i) read one at a time. The output is the sum of all the numbers above the threshold.

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vivado HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `data.h`: define the dataformats
   * `algo.h`, `algo.cpp`: header and source file for the code to be synthethised
* `algo_ref.cpp`: a clean and readable reference implementation of the algorithm, for validation
* `algo_test.cpp`: C++ testbench, that compares the implementation to be synthetised to the reference one to make sure they're bitwise identical (i.e. that any re-writing of the C++ code to make HLS work better didn't change the results)
* `vhdl`: VHDL testbench of the HLS algorithm and independent VHDL implementation of the algorithm itself.

As usual, run with `vivado_hls -f run_hls.tcl`


