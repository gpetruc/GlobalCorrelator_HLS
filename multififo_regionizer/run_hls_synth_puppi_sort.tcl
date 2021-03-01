#source configIP.tcl
set hlsTopFunc sort_pfpuppi_cands
set pfBoard "VCU118"
set pfReg "HGCal"
set hlsIPVersion 1.0
set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} -DROUTER_NOMUX"
# open the project, don't forget to reset
open_project -reset project_puppi_sort

set_top sort_pfpuppi_cands_hybrid
add_files puppi_sort_test.cpp -cflags "${cflags}"

open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
#create_clock -period 2.5 -name default
#create_clock -period 2.2 -name default
create_clock -period 2 -name default
#create_clock -period 1.8 -name default

# just check that the C++ compiles
#csim_design

# synthethize the algorithm
csynth_design

# run the simulation of the synthethized design
#cosim_design -trace_level all

# export this for integration into a firmware design
export_design -format ip_catalog -vendor "cern-cms" -version ${hlsIPVersion} -description "${hlsTopFunc}"

# exit Vivado HLS
exit

# set the FPGA (VU9P on VCU118), and a 360 MHz clock (2.78ns) with some extra margin
# set_part {xcvu9p-flga2104-2L-e}
# create_clock -period 2.7
