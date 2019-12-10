# First tutorial example

An algorithm that computes particle-level jets


# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vivado HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `data.h`: define the dataformats
   * `algo.h`, `algo.cpp`: header and source file for the code to be synthethised
* `algo_ref.cpp`: a clean and readable reference implementation of the algorithm, for validation
* `algo_test.cpp`: C++ testbench, that runs the algorithm. 
* `pfcands_ttbar.txt`: an input file of Puppi candidates and ak4 Puppi jets dumped from CMSSW, used as input for the test bench


# Running the example
## Batch mode using Tcl script
`vivado_hls -f run_hls.tcl`

# Some different levels of optimization in the implementation

Multiple versions of algo.cpp are committed in the source code 
* `algo_v0.cpp`: First attempt, with loop-based `find_seed` function to find the seed: At II=2, Latency 162. Resources: 0 BRAMs, 540 DSPs, 119k FFs, 99k LUTs.

* `algo_v1.cpp`: with recursive-template `find_seed` to reduce the latency but with some increase in resources: At II=2, Latency 90, Resources: 0 BRAMs, 540 DSPs, 89k FFs, 140k LUTs.
* `algo_v4.cpp`: optimization of the implementation: code cleanup, set bit precisions, implement the division with a lookup table (reduce further the latency)
    * At II=2, Latency 52, Resources: 3 BRAMs, 546 DSPs, 90k FFs, 131k LUTs
    * At II=1, Latency 52, Resources: 6 BRAMs, 552 DSPs, 119k FFs, 209k LUTs 
* `algo_v5.cpp`:
   * Optimized seeding using partial sorting to bring the seed as first element in the list, to reduce the candidates to be evaluated at each step. At II=1, Latency 39, Resources: 6 BRAMs, 489 DSPs, 102k FFs, 78k LUTs
* `algo.cpp`:
   * Further optimization from @thesps implementing the seed finding with TreeReduce (logic is similar to v4 but the implementation is better), further reducing LUTs and FFs. DSPs slightly increased as the number of candidates at each iteration remains the same. At II=1, Latency 40, Resources: 6 BRAMs, 507 DSPs, 89k FFs, 68k LUTs
    

