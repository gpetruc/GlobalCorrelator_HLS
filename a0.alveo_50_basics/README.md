# First example for the Alveo

This project demonstrates running a very simple HLS IP (kernel) on the Alveo u50.

The kernel will take two vectors of 16-bit integers, and write the element-wise product in a vector of 32-bit integers.
The kernel will be reading the inputs and writing the outputs from the device memory.

The main differences in preparing the HLS project are:
 * add the option `-flow_target vitis` to open_solution
 * target the `xcu50-fsvh2104-2-e` fpga
 * configure the input and output arrays with `#pragma hls interface mode=m_axi port=x offset=slave bundle=gmem`
 * run `export_design -format xo` after the synthesis
 
 After that, it can be compiled as a binary image for the U50 with
 ```bash
 test -d ~/vitis_ip_cache || mkdir ~/vitis_ip_cache
 v++ --target hw --platform xilinx_u50_gen3x16_xdma_5_202210_1 --link proj/solution/impl/export.xo -o proj.xclbin --remote_ip_cache ~/vitis_ip_cache
 ```
 It takes 1-2h to compile.
 
 
A test example `test.py` shows how to run the test on the Alveo using the python Pynq library.