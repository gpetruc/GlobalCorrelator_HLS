if { [ info exists env(puppiReg) ] } { set puppiReg $env(puppiReg) } { set puppiReg "HGCal" }
if { [ info exists env(puppiBoard) ] } { set puppiBoard $env(puppiBoard) } { set puppiBoard "none" }
if { [ info exists env(sortAlgo) ] } { 
    #set algos { $env(sortAlgo) }  ## doesn't work, and too ignorant on TCL to fix it :-(
    if { $env(sortAlgo) == "bitonic" } { set algos { "bitonic" } }
    if { $env(sortAlgo) == "hybrid" } { set algos { "hybrid" } }
} { 
    set algos { "hybrid" "bitonic" } 
}

foreach algo $algos {

    set cflags "-std=c++0x -DREG_${puppiReg} -DBOARD_${puppiBoard} -DSORT_${algo}" 

    open_project -reset proj_puppi_sort_${algo}_${puppiReg}_${puppiBoard}

    set_top sort_puppi_cands_${algo}
    add_files firmware/puppi_sort_hybrid.cpp -cflags "${cflags}"
    add_files -tb puppi_sort_test.cpp -cflags "${cflags}"
    add_files -tb ../utils/test_utils.cpp  -cflags "${cflags}"
    add_files -tb ../utils/pattern_serializer.cpp -cflags "${cflags}"

    open_solution -reset "solution"
    set_part {xcvu9p-flga2104-2L-e}
    create_clock -period 1.8 -name default
    config_rtl -reset none

    # just check that the C++ compiles
    csim_design

    if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
        csynth_design
        if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
            cosim_design -trace_level all
        }
        if { [info exists env(DO_EXPORT)] && $env(DO_EXPORT) == "1"  } {
            export_design -format ip_catalog -vendor "cern-cms" 
        }
    }

}
exit
