open_project -reset proj
set_top func_many_streams
add_files src/func.cc
add_files -tb testbench.cc

# reset the solution
open_solution -reset "solution" -flow_target vitis
set_part xcu50-fsvh2104-2-e
create_clock -period 2.777
#set_part {xcvu9p-flga2577-2-e}
#create_clock -period 2.777

# just check that the C++ compiles
csim_design -argv "1 1000"
csim_design -argv "2 256"

# synthethize the algorithm
csynth_design

export_design -format xo

quit
