# create a project
open_project -reset "proj"
# specify the name of the function to synthetize
set_top myfunc2
# load source code for synthesis
add_files src/func.cc
# load source code for the testbench
add_files -tb testbench.cc

# create a solution (i.e. a hardware configuration for synthesis)
open_solution -reset "solution"
# set the FPGA (VU9P), and a 320 MHz clock
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 3 

csim_design
csynth_design
exit
