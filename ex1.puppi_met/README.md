# Exercise 1

Write an algorith  to compute the E<sub>T</sub><sup>miss</sup> from all L1 Puppi objects, and the same but excluding muons.

The definition of a L1 Puppi object is defined as a 64 bit word, with the first 40 bits that have the following encoding
| Range | Field | Bits | Format     |
| ------ | ---- | ---- | -----| 
| 13-0  | p<sub>T</sub>    | 14  | unsigned int, LSB = 0.25 GeV |
| 25-14  | &eta;   | 12  | signed int, LSB =  &pi;/720 = &frac14;&deg; |  
| 36-26  | &phi;   | 11  | signed int, LSB = &pi;/720 = &frac14;&deg;  |  
| 39-37 | PID  | 3  | see table below |  
the encoding of the remainig bits different for charged and neutral particles, and are not used in this exercise.

| Val | Binary | Particle | PDG ID |
| ---- | ---- | ---- | ---- |
| 0 | 000 | neutral hadron | 130 (K<sup>0</sup><sub>L</sub>) |
| 1 | 001 | photon | 22 (&gamma;) | \hline
| 2 | 010 | hadron of charge -1 | -211 (&pi;<sup>&minus;</sup>) |
| 3 | 011 | hadron of charge +1 | +211 (&pi;<sup>+</sup>) | \hline
| 4 | 100 | electron | +11 (e<sup>&minus;</sup>)  |
| 5 | 101 | positron | -11 (e<sup>+</sup>) | \hline
| 6 | 110 | muon | +13 (&mu;<sup>&minus;</sup>) |
| 7 | 111 | anti-muon | -13 (&mu;<sup>&minus;</sup>) | \hline

The <sub>T</sub><sup>miss</sup> object should be a 64-bit word with this format
| Bit range | Content | Bits | Encoding | Notes |
| --------- | ------- | ---- | -------- | ----- |
| 0      | valid  | 1    | 0 = null object, 1 = valid object      | |
| 16-1  | E<sub>T</sub><sup>miss</sup>  | 16 | linear, LSB = 31.25 MeV                 | Saturates at 2 TeV |
| 29-17 | &phi; | 13 | two's complement, LSB = 2&pi;/(2<sup>13</sup>) | |
| 45-30  | E<sub>T</sub><sup>tot</sup>  | 16 | linear, LSB = 31.25 MeV                 | Saturates at 2 TeV |
| 63-46 | spare | 20  | &ndash; | &ndash; |
where E<sub>T</sub><sup>tot</sup> is the scalar sum of the object p<sub>T</sub>'s.

In the trigger, the algorithm will take as input a vector of 208 particles, run at 360 MHz with an initialization interval of 54, and return the two sums.

In the Alveo, the algorithm will take as input a number of particles, an input pointer to 64-bit words for the inputs, and an output pointer to 64 bit words for the output.