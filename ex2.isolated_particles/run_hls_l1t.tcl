#exec wget -q -N https://raw.githubusercontent.com/gpetruc/p2-scouting-clients/master/root/data/Puppi.dump -O data/Puppi.dump
exec wget -q -N http://gpetrucc.web.cern.ch/gpetrucc/drop/puppi_TTbar_PU200_1orbit.raw -O data/Puppi.dump
open_project -reset proj_l1t
set_top compute_isolated_l1t
add_files src/algo.cc
add_files -tb testbench.cc -cflags "-DON_L1T" 
add_files -tb data/Puppi.dump

open_solution -reset "solution"
set_part {xcvu9p-flga2577-2-e}
create_clock -period 2.777

csim_design
#csynth_design
exit