#exec wget -q -N https://raw.githubusercontent.com/gpetruc/p2-scouting-clients/master/root/data/Puppi.dump -O data/Puppi.dump
exec wget -q -N http://gpetrucc.web.cern.ch/gpetrucc/drop/puppi_TTbar_PU200_1orbit.raw -O data/Puppi.dump
open_project -reset proj_alveo
set_top compute_sums_alveo
add_files src/algo.cc
add_files -tb testbench.cc -cflags "-DON_ALVEO"
add_files -tb data/Puppi.dump

open_solution -reset "solution" -flow_target vitis
set_part xcu50-fsvh2104-2-e
create_clock -period 3.5

csim_design
csynth_design

export_design -format xo
exit