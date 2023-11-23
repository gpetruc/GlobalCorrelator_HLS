# open the project, don't forget to reset
open_project -reset proj
set_top algo_main
add_files src/algo.cpp
add_files -tb algo_test.cpp 
add_files -tb algo_ref.cpp

# reset the solution
open_solution -reset "solution"
# set the FPGA (VU13P), and a 360 MHz clock
set_part {xcvu13p-flga2577-2-e}
create_clock -period 2.777

# just check that the C++ compiles
csim_design

# synthethize the algorithm
csynth_design

# run the simulation of the synthethized design
#cosim_design -trace_level all

# export this for integration into a firmware design
#export_design -format ip_catalog

# exit Vivado HLS
exit
