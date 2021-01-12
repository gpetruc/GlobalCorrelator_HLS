# get the configuration
set pfBoard "none"
#set pfBoard "VCU118"
set pfReg "Barrel"
set hlsIPVersion 25.4

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard}"

open_project -reset "proj_pf${pfReg}_${pfBoard}"
if { $pfBoard == "none" } {
    set hlsTopFunc pfalgo3
} else {
    set hlsTopFunc packed_pfalgo3
}
set_top ${hlsTopFunc}
add_files firmware/pfalgo3.cpp -cflags "${cflags}"
add_files -tb pfalgo3_test.cpp -cflags "${cflags}"
add_files -tb ref/pfalgo3_ref.cpp  -cflags "${cflags}"
add_files -tb ref/pfalgo_common_ref.cpp  -cflags "${cflags}"
add_files -tb ../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../data/TTbar_PU200_Barrel.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 3.0 -name default

config_interface -trim_dangling_port
# do stuff
csim_design
csynth_design
cosim_design -trace_level all
#export_design -format ip_catalog -vendor "cern-cms" -version ${hlsIPVersion} -description "${hlsTopFunc}"

# exit Vivado HLS
exit
