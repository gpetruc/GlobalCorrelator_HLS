# First tutorial example

A function that reads 3 input arrays `a[i]`, `b[i]`, `c[i]` and computes the sum of `a*b+c`, truncating it in case it would overflow

Three implementations are considered: 
 * `op_int`, `op_int_redux`: using `ap_int` types to store values multiplied by powers of 2, and do bit shits and saturation by hand. 
 * `op_fix`, `op_fix_redux`: using `ap_fixed` types with default saturation rules, to let HLS handle the bit shit and saturation

The implementations with `_redux` in the name implement the summation as a tree reduction with templates: for `ap_int`, HLS can infer the reduction in the sum automatically, while for the `ap_fixed` types this doesn't happen as the saturation and truncation modes could potentially make the operations not associative.


# Running the example

Change the top function in `run_hls.tcl` and run with `vivado_hls -f run_hls.tcl`

Performance:
 * `op_int`: Latency 2, Estimated clock 2.551 ns, DSP 12, FF 551, LUT 716 
 * `op_fix`: Latency 6, Estimated clock 2.533 ns, DSP 12, FF 1109, LUT 863 
 * `op_sat`: Latency 7, Estimated clock 2.760 ns, DSP 12, FF 1233, LUT 2030

 * `op_int_redux`: Latency 2, Estimated clock 2.551 ns, DSP 12, FF 551, LUT 716 (unchanged, as expected)
 * `op_fix_redux`: Latency 2, Estimated clock 2.531 ns, DSP 12, FF 373, LUT 461
 * `op_sat_redux`: Latency 2, Estimated clock 2.626 ns, DSP 12, FF 372, LUT 618

Concluding, `ap_fixed` has good performance but HLS may have more problems unrolling it.
