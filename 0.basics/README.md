# Basics of HLS

This project defines a few simple functions that can be synthethyzed, to demonstrate running Vitis HLS
 * `basic_sum`: a function that adds up two numbers
 * `basic_mul`: a function that multiplies two numbers
 * `pipelined_mul`: a function that multiplies two numbers, pipelined
 * `sum_and_mul`: a function that computes both the sum or the product of two numbers, pipelined
 * `sum_or_mul`: a function that depending on one input computes either the sum or the product of two numbers, pipelined
 * `basic_div`: a division, just to discourage people from doing it
 * `fix_sum`: a sum, using fixed-point numbers
 * `fix_sum_sat`: a sum, using fixed-point numbers with saturation 

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vitis HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `func.h`, `func.cc`: header and source file for the code to be synthethised
* `testbench.cc`: simple C++ testbench that runs the algorithm on a few random input numbers

# Running the example
## Create the Vitis HLS project 
`vitis_hls -f run_hls.tcl`

## Open the project in the GUI
After creating the project using the tcl script (default name from script is `proj`)

`vitis_hls -p proj`

## Running first simulation and synthesis

From the GUI, you can use the toolbar to run the synthesis for this project, and to change the function to be synthetyzed (Project &rarr; Project Settings &rarr; Synthesis &rarr; Top Function).

