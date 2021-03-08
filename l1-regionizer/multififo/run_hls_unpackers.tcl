if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "VCU118" }
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }
set regionizerCFlags "-DROUTER_STREAM -DNO_VALIDATE -DROUTER_ISMUX=1 -DROUTER_ISSTREAM=1 -DNTEST=10";

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} ${regionizerCFlags}"

set funcs { "unpack_track_3to2" "unpack_hgcal_3to1" "unpack_mu_3to12" }
foreach func ${funcs} {
    open_project -reset "project_${func}"
    set_top ${func}

    add_files  firmware/dummy_obj_unpackers.cpp -cflags "${cflags}"
    add_files -tb regionizer_pf_puppi_test_tm18.cpp -cflags "${cflags}"
    add_files -tb tdemux/tdemux_ref.cpp   -cflags "${cflags}"
    add_files -tb utils/tmux18_utils.cpp -cflags "${cflags}"
    add_files -tb utils/dummy_obj_packers.cpp -cflags "${cflags}"
    add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"
    add_files -tb ../../pf/ref/pfalgo_common_ref.cpp   -cflags "${cflags}"
    add_files -tb ../../pf/ref/pfalgo2hgc_ref.cpp   -cflags "${cflags}"
    add_files -tb ../../puppi/linpuppi_ref.cpp   -cflags "${cflags}"
    add_files -tb ../common/regionizer_base_ref.cpp -cflags "${cflags}"
    add_files -tb regionizer_new_ref.cpp -cflags "${cflags}"
    add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
    add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
    add_files -tb ../../data/TTbar_PU200_${pfReg}.dump

    open_solution -reset "solution"
    set_part {xcvu9p-flga2104-2L-e}
    create_clock -period 2.2

    csim_design
    csynth_design
    if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
        cosim_design -trace_level all
    }
}

exit
