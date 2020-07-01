# create a project
open_project -reset "proj"
# specify the name of the function to synthetize
set_top tdemux_simple
# load source code for synthesis
add_files src/tdemux.cc
# load source code for the testbench
#add_files -tb tdemux_ref.cc
add_files -tb testbench_tdemux.cc

# create a solution (i.e. a hardware configuration for synthesis)
open_solution -reset "solution"
# set the FPGA (VU9P), and a 320 MHz clock
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 3.125 

# end here, so that we can then open the project interactively in the gui
csim_design
csynth_design
#cosim_design -trace_level all
exit
