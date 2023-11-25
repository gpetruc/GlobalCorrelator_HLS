# Introduction to arrays in HLS

A very simple example: a function that reads two input arrays `a[i]`, `b[i]`, and computes the products (dropping the 8 least siginficant bits), and adds them up.

It's implemented in 3 version:
 * `mul_add_basic`: no HLS-specific annotations
 * `mul_add_pipelined`: adding `#pragma HLS pipeline II=1`
 * `mul_add_partitioned`: adding `#pragma HLS array_partition variable=X complete` for both input arrays

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vitis HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `func.h`, `func.cc`: header and source file for the code to be synthethised
* `testbench.cc`: simple C++ testbench that runs the algorithm on a few random input files

# Running the example
## Create the Vitis HLS project 
`vitis_hls -f run_hls.tcl`

## Open the project in the GUI
After creating the project using the tcl script (default name from script is `proj`)

`vitis_hls -p proj`

## Running first simulation and synthesis

From the GUI, you can use the toolbar to run the synthesis for this poject.
You should see that the algorithm implements the input and output vectors as BRAMs, and the latency is 17 clock cycles. 
Only 1 DSP multiplier is used.

## Pipelining the function

With `#pragma HLS pipeline II=1`, Vitis will try to try pipeline the function.
Vitis will also implicitly infer that it has to unroll the loop.
However, it will complain that the requested II=1 cannot be achieved as the input BRAMs don't have enough throughput: with 2 ports, it takes 6 clock cycles to read the full set of input data, and thus the best that can be achieved is II=6.

## Partitioning arrays

The `#pragma HLS array_partition` directive allows reading all elements simultaneously.
Now the pipelining is successful and the algorithm has a latency of 2 clock cycles, II=1.
12 DSPs are needed for the algorithm, since it needs 12 multiplications per clock cycle.
