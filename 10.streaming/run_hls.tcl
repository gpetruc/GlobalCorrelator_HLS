############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2016 Xilinx, Inc. All Rights Reserved.
############################################################

# open the project, don't forget to reset
open_project -reset proj
set_top algo_main
add_files src/algo.cpp
add_files -tb algo_test.cpp 
add_files -tb algo_ref.cpp

# reset the solution
open_solution -reset "solution1"
###  MP7 (Virtex-7 690T)
# set_part {xc7vx690tffg1927-2}
##   VCU118 dev kit (VU9P)
set_part {xcvu9p-flga2104-2L-e}
##   Serenity with KU115 
# set_part {xcku115-flvf1924-2-i}
## 240 MHz
create_clock -period 4.16667 -name default

# just check that the C++ compiles
csim_design

# synthethize the algorithm
#csynth_design

# run the simulation of the synthethized design
#cosim_design -trace_level all

# export this for integration into a firmware design
#export_design -format ip_catalog

# exit Vivado HLS
exit
