# get the configuration
if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "none" }
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
add_files -tb ../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb ../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../data/TTbar_PU200_${pfReg}.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 3.0 -name default

config_interface -trim_dangling_port
# do stuff
csim_design
if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
    csynth_design
    if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
        cosim_design -trace_level all
    }
    if { [info exists env(DO_EXPORT)] && $env(DO_EXPORT) == "1"  } {
        export_design -format ip_catalog -vendor "cern-cms" -version ${hlsIPVersion}
    }
}

# exit Vivado HLS
exit
