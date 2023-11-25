# A first physics example: H<sub>T</sub>

Second tutorial example: an algorithm that computes the scalar sum p<sub>T</sub> of all the objects with |&eta;| &lt; 2.4
 * define a simple structure to hold a "particle" object with a p<sub>T</sub> and an &eta; value, stored as integer
 * define a reference implementation
 * try different C++ implementatons for synthesis that result in different performances


# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vitis HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `data.h`: define the dataformats
   * `algo.h`, `algo.cpp`: header and source file for the code to be synthethised
* `algo_ref.cpp`: a clean and readable reference implementation of the algorithm, for validation
* `algo_test.cpp`: C++ testbench, that compares the implementation to be synthetised to the reference one to make sure they're bitwise identical (i.e. that any re-writing of the C++ code to make HLS work better didn't change the results)

# Running the example
## Batch mode using Tcl script
`vivado_hls -f run_hls.tcl`

In this case, the Tcl script is configured to also run the synthesis. 
The report of the synthesis is saved in `proj/solution/syn/report/algo_main_csynth.rpt`

## Synthesis results (Vitis 2023.1)

For the first implementation, it has a latency of 13 clock cycles, using 2629 FFs and 2385 LUTs.
Vitis is not understanding that it can parallelize the sum, apparently because of the if inside the loop

Changing the loop code so that the sum is always executed, but we add zero in some entries, makes HLS understand that the code can be parallelized.
Now the loop takes 2 clocks (5.6ns), 375 FFs, 1271 LUTs.

An alternative approach to force Vitis HLS to do parallelize the reduction is to use recursive templates, which works also in more general cases of divide-and-conquer algorithms (e.g. finding the maximum or minimum value, sorting, ...). The result is identical, 2 clocks (5.6ns), 375 FFs, 1271 LUTs.

## Changing the clock frequency

We can change the clock period to see how it affects the results, from the TCL file or from the solution settings in the GUI (or adding new solutions, from the GUI):
 * 480 MHz (2.08ns): 3 clock cycles (6.2ns), 580 FF, 1271 LUTs
 * 240 MHz (4.16ns): 1 clock cycle (4.2ns), 156 FF, 1271 LUTs

The amount of computations needed, and thus of LUTs remains the same, and the latency in ns remains similar (slightly higher).
At higher speed, less LUT operations can be done per clock cycle and so more clock cycles are needed, and consequently more FFs (and more FPGA interconnect resources, not shown by HLS)




