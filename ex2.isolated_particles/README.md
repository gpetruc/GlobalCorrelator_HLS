# Exercise 2

Write an algorith to reconstruct high p<sub>T</sub> isolated charged particles starting from the list of all particles (L1 Puppi objects) in the event.
We will define as "isolated" a particle if the sum  p<sub>T</sub> of all other particles around it in a cone of &Delta;R < 0.4 (excluding an inner veto cone &Delta;R = 0.01) is less than the particle p<sub>T<sub>

The definition of a L1 Puppi object is defined as a 64 bit word, with the first 40 bits that have the following encoding
| Range | Field | Bits | Format     |
| ------ | ---- | ---- | -----| 
| 13-0  | p<sub>T</sub>    | 14  | unsigned int, LSB = 0.25 GeV |
| 25-14  | &eta;   | 12  | signed int, LSB =  &pi;/720 = &frac14;&deg; |  
| 36-26  | &phi;   | 11  | signed int, LSB = &pi;/720 = &frac14;&deg;  |  
| 39-37 | PID  | 3  | see table below |  

the encoding of the remainig bits different for charged and neutral particles, and are not used in this exercise.

| PID | Binary | Particle | PDG ID |
| ---- | ---- | ---- | ---- |
| 0 | 000 | neutral hadron | 130 (K<sup>0</sup><sub>L</sub>) |
| 1 | 001 | photon | 22 (&gamma;) | \hline
| 2 | 010 | hadron of charge -1 | -211 (&pi;<sup>&minus;</sup>) |
| 3 | 011 | hadron of charge +1 | +211 (&pi;<sup>+</sup>) | \hline
| 4 | 100 | electron | +11 (e<sup>&minus;</sup>)  |
| 5 | 101 | positron | -11 (e<sup>+</sup>) | \hline
| 6 | 110 | muon | +13 (&mu;<sup>&minus;</sup>) |
| 7 | 111 | anti-muon | -13 (&mu;<sup>+</sup>) | \hline

Isolated particles can be found with an iterative algorithm:
 * at start, mark all charged particles as potential seeds
 * select as seed the highest p<sub>T</sub> charged particle that hasn't been masked yet
 * compute the &Delta;R&sup2; to all other particles in the event (either masked or unmasked)
   * add up the p<sub>T</sub> of all the particles in the cone if outside of the veto cone
   * mask off all particles within the isolation cone (including the seed)
 * if the new p<sub>T</sub> particle is isolated, add it to the output list.
 * repeat

 The output should be a list of N particles (e.g. N=12)

In the trigger, the algorithm will take as input a vector of 208 particles, run at 360 MHz with an initialization interval of 54, and return a vector of isolated particles (L1 Puppi objects) and the associated vector of absolute isolation values (same data type as p<sub>T</sub>)..

In the Alveo, the algorithm will take as input a number of particles, the number of iterations, an input pointer to 64-bit words for the inputs, and output pointer to 64 bit and 16 bit words for the output.