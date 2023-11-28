# Second example for the Alveo: parallel streams

This project demonstrates running 64 parallel streams of computation on an Alveo.
Each stream takes as input an integer N computes the sum of the cubes of the numbers up to N.

We use the `hls::stream<T>` class, that words as a FIFO queue (first-in, first-out): 
 * in the top-level function we put tasks, that in our case are just values on N, into the stream
 * in each task, we read from the input stream, process and write into the output stream
 * then, we read from the all the output streams to assemble the result
