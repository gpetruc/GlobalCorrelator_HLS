# VHDL implementation of the same algorithm

## Algorithm

The VHDL source code for the algorithm is in file `algo.vhd`.

The interface of the algorithm block is similar to that out of HLS, with
 * `clk`, `rst`: clock and asynchronous reset
 * `threshold_in`, `value_in`: inputs for the threshold and the values
 * `event_in`: input, to be set to true for the first value in the event and false otherwise
 * `sum_out`:  output, the sum of the numbers above threshold
 * `out_valid`: output, true when `sum_out` contents are good

Implementation constraints:
 * The number of input objects is fixed to `NITEMS` (6) in all events. 
 * The algorithm starts counting properly when `event_in` is turned on for the first time after a reset. After that, new valid data must be provided at each clock, and `event_in` must be true once every `NITEMS` clocks, otherwise `out_valid` will report incorrectly.
 * The `threshold_in` input must stay good for the full set of `NITEMS` input items (it's not buffered inside the algo)
 * The output stays valid only for one clock cycle.

This file can in principle be synthetized to firmware, but I haven't checked timing and resources.

## Simulation Test bench
A VHDL simulation test bench for the VHDL algorithm is provided in `algo_tb.vhd`.

This test bench reads inputs from the text files produced by the HLS C simulation, runs behavioural VHDL simulation using Vivado's XSim simulator, and dumps the output into a text file.

A script to compile the VHDL code and run the behavioural simulation is provided, and can be run as

`./run.sh [ --gui ]`

If run with the `--gui` option, it opens the Vivado simulation GUI allowing to display the waveforms of the simulated signals. If not, it runs in batch and the results can be studied from the otput text files.


# VHDL testbench for the HLS IP core

A separate test bench for the HLS IP core is provided in `algo_tb_hlsipcore.vhd`. This feeds the same inputs as the testbench of the VHDL-only algorithm to the IP core.

The internal item counter of the HLS IP core starts counting when `ap_start` is first asserted to `1`, and `ap_done` signals when the block has processed all the inputs.
The `ap_fifo` input protocol for the stream is used, with the `*_empty_n` tied to `1` (the fifo is never empty), and `*_read` reported in the testbench but not really used (we assume the reads are always successful, so we always provide a new input at the next clock)

The testbench can be run from `run_hlsipcore.sh`  with the same options as `run.sh`. The IP core is taken from its synthethized VHDL version, not packaged.
