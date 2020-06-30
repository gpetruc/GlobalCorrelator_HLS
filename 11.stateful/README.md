# Stateful constructs 

Create some firmware blocks that have a valid internal state.

## `multitap_sr`: variants of multitap shift registers (serial-in, parallel-out)

A few diffent elements are implemented in `multitap_sr.{h,cc}`, `multitap_ref.cc`, `run_hls_multitap.sh`

 * `multitap_sr_push_simple`: just keep the last N values in the order they were received; inputs are the new value, and a boolean signaling it's a new event and thus the shift register should be cleared before inserting the new elements.
   * implemented as function with simple inputs (no hls streams), keeping state in static variables inside, synthetized pipelined at #II=1. Latency 0 (estimated clock 1.6 ns), 310 FF, 335 LUTs
 * `sorting_multitap_sr_push_simple`: same inputs as before, but the register holds a sorted list of the higest N inputs received.
   * implemented as function with simple inputs (no hls streams), keeping state in static variables inside, synthetized pipelined at #II=1. Latency 1 (estimated clock 1.95 ns), 973 FF, 1692 LUTs
