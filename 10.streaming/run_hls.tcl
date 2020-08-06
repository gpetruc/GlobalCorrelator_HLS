# open the project, don't forget to reset
open_project -reset project
set_top algo_main
add_files src/algo.cpp
add_files -tb algo_test.cpp 
add_files -tb algo_ref.cpp

set cflags "-std=c++0x"

# reset the solution
open_solution -reset "solution"
##   VCU118 dev kit (VU9P)
set_part {xcvu9p-flga2104-2L-e}
## target 360 MHz (2.78ns) with some margin
create_clock -period 2.5 -name default

# just check that the C++ compiles
csim_design

# synthethize the algorithm
csynth_design

# run the simulation of the synthethized design
cosim_design -trace_level all

# export this for integration into a firmware design
export_design -format ip_catalog

# exit Vivado HLS
exit
