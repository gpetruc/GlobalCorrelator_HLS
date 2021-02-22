source configIP.tcl

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} -DROUTER_NOSTREAM -DNO_VALIDATE"
open_project -reset "project_csim_mux"

set_top ${hlsTopFunc}

add_files firmware/calo_regionizer.cpp -cflags "${cflags}"
add_files firmware/tk_regionizer.cpp -cflags "${cflags}"
add_files firmware/mu_regionizer.cpp -cflags "${cflags}"
add_files -tb regionizer_ref.cpp -cflags "${cflags}"
add_files -tb regionizer_new_ref.cpp -cflags "${cflags}"
add_files -tb regionizer_test.cpp -cflags "${cflags}"
add_files -tb ../common/regionizer_base_ref.cpp -cflags "${cflags}"
add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../../data/TTbar_PU200_HGCal.dump

open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.5

csim_design
exit
