# Stateful algorithms

While all the previous examples were stateless data-processing functions, that take an input and compute an output with no side-effects, it is possible to create also stateful algorithms.

This example will be a decoder for the Phase-2 track-matched muons (TkMu) as they are sent to the Global Trigger (GT).
An event sent to the GT is a packet of 64-bit words, received one per clock cycle at 360 MHz. In addition to the 64 data bits, there are single-bit control signals that mark the start and end of the packet.

TkMu objects are written as 96 bit objects, in the following format
| Bit range | Content | Bits | Encoding | Notes | 
| --------- | ------- | ---- | -------- | ----- | 
| 0      | valid  | 1    | 0 = null object, 1 = valid object      | |
| 16-1  | p<sub>T</sub>  | 16 | linear, LSB = 31.25 MeV                 | Saturates at 2 TeV |
| 29-17 | &phi; | 13 | two's complement, LSB = 2&pi;/(2<sup>13</sup>) | |
| 43-30 | &eta; | 14 | two's complement, LSB = 2&pi;/(2<sup>13</sup>) | |
| 53-44 | z<sub>0</sub>  | 10 | two's complement, LSB = 0.05\,cm on &plusmn;25.6 cm | Same as track trigger |
| 63-54 | d<sub>0</sub>  | 10 | two's complement, LSB = 0.03\,cm on &plusmn;15.36 cm | Reduced to 10 bits |
|  64      | q         | 1  | 0 = positive, 1 = negative | |
|  72-65   | Q         | 8  |                            | Track-match related |
|  76-73  | Isolation | 4  | undefined, ignore for now  |  |
|  80-77 | &beta;   | 4  | 16 bins, LSB = 6\%         | Linear increments in [0,1] |
|  95-81 | spare     | 15 |                            | Reserved |
  
The GMT sends 12 TkMuon objects, using 18 64-bit words, with the following format:
| Word | Bits | Content  |
| ---- | ---- | -------- |
|  0   | 63-0 | TkMu 0 bits 63-0 |
|  1   | 31-0 | TkMu 0 bits 96-64 |
|  1   | 63-32 | TkMu 1 bits 96-64 |
|  2   | 63-0 | TkMu 1 bits 63-0 |
|  3   | 63-0 | TkMu 2 bits 63-0 |
|  4   | 31-0 | TkMu 2 bits 96-64 |
|  4   | 63-32 | TkMu 3 bits 96-64 |
|  5   | 63-0 | TkMu 3 bits 63-0 |
| ...  | ...  | ... |
| 16   | 31-0 | TkMu 10 bits 96-64 |
| 16   | 63-32 | TkMu 11 bits 96-64 |
| 17   | 63-0 | TkMu 11 bits 63-0 |

We will write a module that at each event receives at each clock a 64-bit data word and the start-of-event signal, and returns as output a list of 12 TkMu objects, and a bit specifying whether the list for this event complete and can be read by the downstream firmware.

# Structure of this example

* `run_hls.tcl`: configuration & startup file for Vitis HLS defining the project, the input files, etc
* `src` directory with the header file and implementation for the synthesis (i.e. to to be compiled into firmware)
   * `data.h`: a definition for the TkMuon object
   * `func.h`, `func.cc`: header and source file for the code to be synthethised
* `testbench.cc`: simple C++ testbench that runs the algorithm on a few random input numbers

# Running the example

`vitis_hls -f run_hls.tcl`
