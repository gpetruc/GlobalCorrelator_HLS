# Deregionizer
The purpose of the Deregionizer is to reshape the data that arrives at the Correlator Layer 1 from Correlator Layer 2 into a flat array with the particles in the event accessible in parallel on a single cycle.
Given the multiplicity observed in high occupancy events, the number of particles in the event is capped at 128, though this may be increased in future for robustness.

The image shows the sparse data packet that arrives from Layer 1 for an example event of TTbar with 200 PU, and the flat array output by the deregionizer.

The 'merging' involves finding neighbouring continuous blocks of valid particles, then rotating the array of one neighbour to slot exactly next to the other neighbour forming a longer continous block.
The expected structure of the Layer 1 data packet is used to help find continous blocks, as shown in the image.
A single PF region is assumed to send up to 18 particles, pT sorted, sent over 3 clock cycles on 6 links at 25 Gb/s.
Merging is done sequentially on pairs of input regions.
After the 6 PF regions which are received in parallel are merged, there is an accumulation step over the duration of the TM Period to merge all PF regions of the event.
The Accumulate step uses the same principle as the Merge, but rather than merging two neighbouring lists, merges the new list into the old list, reset at the end of the event.

The firmware uses the Reusable Firmware Libary (RUFL) to define `Vector` types and common components. 
The `DataValid` and `FrameValid` flags are made use of extensively to control which inputs are sent to the final list. 
In this context, `DataValid` is `TRUE` for any valid particle (set to any with pT > 0), and `FALSE` otherwise, while `FrameValid` is `TRUE` on any clock cycle where there could be a particle with `DataValid` equal to `TRUE`, connected to the valid bit of the link data, and used to signal any gap in events. 
The input is expected to transmit continuously with no gaps, so `FrameValid` is `TRUE` for all inputs during running.
At the output of the deregionizer, `FrameValid` will be `TRUE` on one clock cycle in the TM Period for all 128 elements in the event particle array, with `DataValid` set to `TRUE` only for particles with `pT > 0`.
Given the continuous arrival of data, an internal counter is used to determine when one event ends and the next begins.
If `FrameValid` of the input goes low, the counter is reset. 

The RUFL source files need to be added to different VHDL libraries for each DataType (`Int`, and `IO`) in the project. Since Vivado only allows a source file to be used in one VHDL library, the source files need to be duplicated. This is implemented using relative symlinks to the RUFL components, so the RuflCore needs to be cloned.
The expected relative position of `RuflCore` to `correlator-common` is the standard ipbb structure, such that `RuflCore/` and `correlator-common/` are in the same directory.
## Using the Deregionizer

To include the deregionizer in ipbb projects (e.g. for emp-fwk & Serenity), in the project `.dep` file include:

`include -c correlator-common:l2-deregionizer deregionizer.dep`

To include the deregionizer in ruckus project (for APx), use the `ruckus.tcl` file at `l2-derionizer/ruckus/ruckus.tcl`. This part is a work in progress, and some changes need to be made to RUFL to work more nicely with Vivado simulator. Although in principle a bitfile could still be made including the deregionizer for APx.

# Deregionizer only projects
The `standalone` directory contains extra modules to create an emp-fwk simulation or Vivado project of the deregionizer only.
The payload outputs the 128 particles of the event on 16 links over 8 clock cycles.
In addition, the output of one module from the first and second Merge layers are send on extra links for debugging purposes.

The `link_map.vhd` module maps physical links of the FPGA, chosen to help with placement and meeting timing, into a more convenient internal indexing.


## One-time ipbb workspace setup
Using `git clone`
```
ipbb init my-ipbb-workspace
cd src/
git clone https://gitlab.cern.ch/p2-xware/firmware/emp-fwk.git -b v0.3.6
git clone https://gitlab.cern.ch/ttc/legacy_ttc.git -b v2.1
git clone https://github.com/ipbus/ipbus-firmware -b v1.8
git clone https://gitlab.cern.ch/rufl/RuflCore.git
git clone https://gitlab.cern.ch/cms-cactus/phase2/firmware/correlator-common.git
```
Using `ipbb add`
```
ipbb init my-ipbb-workspace
ipbb add git https://gitlab.cern.ch/p2-xware/firmware/emp-fwk.git -b v0.3.6
ipbb add git https://gitlab.cern.ch/ttc/legacy_ttc.git -b v2.1
ipbb add git https://github.com/ipbus/ipbus-firmware -b v1.8
ipbb add git https://gitlab.cern.ch/rufl/RuflCore.git
ipbb add git https://gitlab.cern.ch/cms-cactus/phase2/firmware/correlator-common.git
```

## Create an emp-fwk simulation project (Modelsim)
From the directory `my-ipbb-workspace/`:
```
ipbb proj create vivado deregionizer-vcu118 correlator-common:l2-deregionizer/standalone -t deregionizer.dep
cd proj/deregionizer-vcu118
ipbb sim make-project
```

## Create an emp-fwk Vivado project for VCU118 dev kit
From the directory `my-ipbb-workspace/`:
```
ipbb proj create vivado deregionizer-vcu118 correlator-common:l2-deregionizer/standalone -t deregionizer.dep
cd proj/deregionizer-vcu118
ipbb vivado make-project -c
ipbb vivado synth -j8 impl -j8 package
```
