open_project -reset proj
set_top func
add_files src/func.cc
add_files -tb testbench.cc

# reset the solution
open_solution -reset "solution" -flow_target vitis
set_part xcu50-fsvh2104-2-e
create_clock -period 3.5

# just check that the C++ compiles
csim_design -argv "10 30"

# synthethize the algorithm
csynth_design

export_design -format xo

quit
