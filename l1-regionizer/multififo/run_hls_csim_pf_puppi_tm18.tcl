if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "VCU118" }
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }
#set regionizerCFlags "-DROUTER_NOSTREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=0";
#set regionizerCFlags "-DROUTER_STREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=1";
set regionizerCFlags "-DROUTER_STREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=1 -DFAKE_PUPPI";

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} ${regionizerCFlags}"
open_project -reset "project_csim_${pfReg}_pf_puppi_tm18"

set sample TTbar_PU200

add_files -tb regionizer_pf_puppi_test_tm18.cpp -cflags "${cflags}"
add_files -tb tdemux/tdemux_ref.cpp   -cflags "${cflags}"
add_files -tb tdemux/firmware/tdemux.cpp   -cflags "${cflags}"
add_files -tb utils/tmux18_utils.cpp -cflags "${cflags}"
add_files -tb firmware/dummy_obj_unpackers.cpp -cflags "${cflags}"
add_files -tb utils/dummy_obj_packers.cpp -cflags "${cflags}"
add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb ../../pf/ref/pfalgo_common_ref.cpp   -cflags "${cflags}"
add_files -tb ../../pf/ref/pfalgo2hgc_ref.cpp   -cflags "${cflags}"
add_files -tb ../../puppi/linpuppi_ref.cpp   -cflags "${cflags}"
add_files -tb ../common/regionizer_base_ref.cpp -cflags "${cflags}"
add_files -tb multififo_regionizer_ref.cpp -cflags "${cflags}"
add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../../data/${sample}_${pfReg}.dump

open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.5

csim_design
exit
