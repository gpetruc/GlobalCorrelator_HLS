#!/bin/bash
CSIM=../proj/solution1/csim/build
if test -f $CSIM/input.txt; then
    echo " ## Getting C simulation inputs from $CSIM";
    cp -v $CSIM/*.txt .
else
    echo "Couldn't find C simulation inputs in $CSIM.";
    echo "Run vivado_hls -f run_hls.tcl in the parent directory before.";
    exit 1;
fi;

# cleanup
rm -r xsim* xelab* webtalk* vivado* xvhdl* test.wdb output.txt 2> /dev/null || true;

echo " ## Compiling VHDL files: ";
for V in algo.vhd algo_tb.vhd; do
    xvhdl $V || exit 2;
    grep -q ERROR xvhdl.log && exit 2;
done;

echo " ## Elaborating: ";
xelab testbench -s test -debug all || exit 3;
grep -q ERROR xelab.log && exit 3;

if [[ "$1" == "--gui" ]]; then
    echo " ## Running simulation in the GUI: ";
    xsim test --gui
else
    echo " ## Running simulation in batch mode: ";
    xsim test -R || exit 4;
    grep -q ERROR xsim.log && exit 4;

    test -f output.txt && echo " ## Output produced in output.txt ";
fi
