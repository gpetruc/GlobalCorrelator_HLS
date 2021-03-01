#!/bin/bash

if [[ "$1" == "" ]]; then 
    echo "Usage : $0 project"; 
    exit 1; 
fi;
PROJ=$1; shift;

FW="../hdl"
HLS="../../.."
if [[ "${PROJ}" == "vhdl-nomux-tk" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/tk_router_element.vhd ${FW}/tk_router.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge3.vhd"
    VHDLS="${VHDLS} ${FW}/tk_regionizer.vhd tk_regionizer_vhdl_tb.vhd"
    HLSPROJ="project_csim_HGCal_nomux"
    DET="tk"
elif [[ "${PROJ}" == "vhdl-nomux-calo" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/calo_router.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge2.vhd "
    VHDLS="${VHDLS} ${FW}/calo_regionizer.vhd calo_regionizer_vhdl_tb.vhd"
    HLSPROJ="project_csim_HGCal_nomux"
    DET="calo"
elif [[ "${PROJ}" == "vhdl-nomux-mu" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/mu_router.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2.vhd "
    VHDLS="${VHDLS} ${FW}/mu_regionizer.vhd mu_regionizer_vhdl_tb.vhd"
    HLSPROJ="project_csim_HGCal_nomux"
    DET="mu"
#elif [[ "${PROJ}" == "hls_slices" ]]; then
#    VHDLS="$VHDLS project_full_input/solution/syn/vhdl/route_link2fifo.vhd  project_full_input/solution/syn/vhdl/router_input_slice.vhd"
#    VHDLS="$VHDLS project_full_fifo/solution/syn/vhdl/router_fifo_slice_fifos_data_V_0.vhd  project_full_fifo/solution/syn/vhdl/router_fifo_slice.vhd"
#    VHDLS="$VHDLS project_full_merge2/solution/syn/vhdl/router_merge2_slice.vhd"
#    VHDLS="$VHDLS project_full_merge3/solution/syn/vhdl/router_merge3_slice.vhd"
#    VHDLS="$VHDLS project_full_output/solution/syn/vhdl/router_full_output_slice.vhd"
#    VHDLS="$VHDLS ${FW}/regionizer_data_stdlogic.vhd ${FW}/phi_regionizer_hls_slices.vhd phi_regionizer_hls_tb.vhd"
#elif [[ "${PROJ}" == "calo_hls_slices" ]]; then
#    VHDLS="$VHDLS project_MC_calo_input/solution/syn/vhdl/calo_router_input_slice.vhd"
#    VHDLS="$VHDLS project_MC_calo_fifo/solution/syn/vhdl/calo_router_fifo_slice_fifos_data_V_0.vhd"
#    VHDLS="$VHDLS project_MC_calo_fifo/solution/syn/vhdl/calo_router_fifo_slice.vhd"
#    VHDLS="$VHDLS project_MC_calo_merge2/solution/syn/vhdl/calo_router_merge2_slice.vhd"
#    VHDLS="$VHDLS project_MC_calo_merge4/solution/syn/vhdl/calo_router_merge4_slice.vhd"
#    VHDLS="$VHDLS project_MC_calo_merge/solution/syn/vhdl/calo_router_merge_slice.vhd"
#    VHDLS="$VHDLS project_MC_calo_output/solution/syn/vhdl/calo_router_full_output_slice.vhd"
#    VHDLS="$VHDLS ${FW}/regionizer_data_stdlogic.vhd ${FW}/calo_phi_regionizer_hls_slices.vhd calo_phi_regionizer_hls_slices_tb.vhd"
#    HLSPROJ="project_MC_calo_input"
elif [[ "${PROJ}" == "vhdl-mux-tk" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/tk_router_element.vhd ${FW}/tk_router.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge3.vhd"
    VHDLS="${VHDLS} ${FW}/tk_regionizer.vhd ${FW}/stream_sort.vhd ${FW}/region_mux.vhd ${FW}/tk_regionizer_mux.vhd tk_regionizer_vhdl_mux_tb.vhd"
    HLSPROJ="project_csim_HGCal_mux"
    DET="tk"
elif [[ "${PROJ}" == "vhdl-mux-all" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge3.vhd ${FW}/stream_sort.vhd ${FW}/region_mux.vhd"
    VHDLS="${VHDLS} ${FW}/tk_router_element.vhd ${FW}/tk_router.vhd ${FW}/tk_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/calo_router.vhd ${FW}/calo_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/mu_router.vhd ${FW}/mu_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/full_regionizer_mux.vhd"
    VHDLS="${VHDLS} pattern_textio.vhd full_regionizer_vhdl_mux_tb.vhd"
    HLSPROJ="project_csim_HGCal_mux"
    DET="all"
elif [[ "${PROJ}" == "vhdl-stream-tk" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/tk_router_element.vhd ${FW}/tk_router.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge3.vhd"
    VHDLS="${VHDLS} ${FW}/stream_sort.vhd  ${FW}/cascade_stream_sort_elem.vhd  ${FW}/cascade_stream_sort.vhd ${FW}/region_mux_stream.vhd ${FW}/delay_sort_mux_stream.vhd"
    VHDLS="${VHDLS} ${FW}/tk_regionizer.vhd ${FW}/tk_regionizer_mux_stream.vhd tk_regionizer_vhdl_mux_stream_tb.vhd"
    HLSPROJ="project_csim_HGCal_stream"
    DET="tk"
elif [[ "${PROJ}" == "vhdl-stream-all" ]]; then
    VHDLS="${FW}/regionizer_data.vhd ${FW}/rolling_fifo.vhd ${FW}/fifo_merge2.vhd ${FW}/fifo_merge2_full.vhd ${FW}/fifo_merge3.vhd "
    VHDLS="${VHDLS} ${FW}/stream_sort.vhd  ${FW}/cascade_stream_sort_elem.vhd  ${FW}/cascade_stream_sort.vhd ${FW}/region_mux_stream.vhd ${FW}/delay_sort_mux_stream.vhd"
    VHDLS="${VHDLS} ${FW}/tk_router_element.vhd ${FW}/tk_router.vhd ${FW}/tk_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/calo_router.vhd ${FW}/calo_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/mu_router.vhd ${FW}/mu_regionizer.vhd "
    VHDLS="${VHDLS} ${FW}/full_regionizer_mux_stream.vhd"
    VHDLS="${VHDLS} pattern_textio.vhd full_regionizer_vhdl_mux_stream_tb.vhd"
    HLSPROJ="project_csim_HGCal_stream"
    DET="all"
else
    echo "Unknown project $PROJ";
    exit 1;
fi

SIMULATOR="xsim"
if [ -d ${PROJ}_${SIMULATOR}.dir ]; then
    echo "Re-creating ${PROJ}_${SIMULATOR}.dir directory"
    rm -r ${PROJ}_${SIMULATOR}.dir || exit 1;
else
    echo "Creating new ${PROJ}_${SIMULATOR}.dir directory"
fi;
mkdir ${PROJ}_${SIMULATOR}.dir || exit 1;
cd ${PROJ}_${SIMULATOR}.dir || exit 1;

CSIM=../$HLS/$HLSPROJ/solution/csim/build
if test -f $CSIM/input-${DET/all/tk}.txt; then
    echo " ## Getting C simulation inputs from $CSIM";
    cp -v $CSIM/*-${DET/all/*}.txt .
else
    echo "Couldn't find C simulation inputs in $CSIM.";
    echo "Run vivado_hls in the parent directory before.";
    exit 1;
fi;

# cleanup
rm -r xsim* xelab* webtalk* xvhdl* test.wdb 2> /dev/null || true;

echo " ## Compiling VHDL files: $VHDLS";
for V in $VHDLS; do
    xvhdl ../${V} || exit 2;
done;

echo " ## Elaborating: ";
xelab testbench -s test -debug all || exit 3;

if [[ "$1" == "--gui" ]]; then
    echo " ## Running simulation in the GUI: ";
    xsim test --gui
else
    echo " ## Running simulation in batch mode: ";
    xsim test -R || exit 4;
fi
