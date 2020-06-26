# First tutorial example

A very simple example: a function that reads two input arrays `a[i]`, `b[i]`, and computes their element-wise product 'c[i]' and the sum of all.

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vivado HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `func.h`, `func.cc`: header and source file for the code to be synthethised
* `testbench.cc`: simple C++ testbench that runs the algorithm on a few random input files

# Running the example
## Create the Vivado HLS project 
`vivado_hls -f run_hls.tcl`

## Open the project in the GUI
After creating the project using the tcl script (default name from script is `proj`)

`vivado_hls -p proj`

## Running first simulation and synthesis

From the GUI, you can use the toolbar to run the synthesis for this poject.
You should see that the algorithm implements the input and output vectors as BRAMs, and the latency is 49 clock cycles. 
The loop is executed sequentially, and each iteration of the loop takes 4 cycles.
Only 1 DSP multiplier is used.

## Pipelining the function

Uncomment the `#pragma HLS pipeline II=1` in `func.cc` to tell Vivado to try pipeline the function.
Vivado will also infer that it has to unroll the loop.
However, it will complain that the requested II=1 cannot be achieved as the input BRAMs don't have enough throughput: with 2 ports, it takes 6 clock cycles to read the full set of input data, and thus the best that can be achieved is II=6.

## Partitioning arrays

Uncomment also the `#pragma HLS array_partition` directives to allow reading all elements simultaneously.
Now the pipelining is successful and the algorithm has a latency of 2 clock cycles, II=1.
12 DSPs are needed for the algorithm, since it needs 12 multiplications per clock cycle.
