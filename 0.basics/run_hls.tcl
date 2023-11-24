# create a project
open_project -reset "proj"
# specify the name of the function to synthetize
set_top basic_sum
# load source code for synthesis
add_files src/func.cc
# load source code for the testbench
add_files -tb testbench.cc

# create a solution (i.e. a hardware configuration for synthesis)
open_solution -reset "solution"
# set the FPGA (VU9P), and a 360 MHz clock
set_part {xcvu9p-flga2577-2-e}
create_clock -period 2.777

# end here, so that we can then open the project interactively in the gui
exit
