List of things to do
TODO: implement multicore processing
TODO: implement algorithm for multicore (https://ieeexplore.ieee.org/document/6904264)
TODO: add resources processes can use
TODO: add deadlock detection/prevention


Original format:
arrivalTime reqProcessorTime ioTime ioDur ioTime ioDur ...

New format:



NOTES:
algorithm for multicore:
task migration (from one processor to another) is not an issue since this simulation
does not take into account the cache relative to each processor
The algorithm we use can ignore the inneficiancies of process migration.
The algorithm can prioritize load balancing over processor affinity
Normally, this is a main problem related to multicore design.

I'll do a multicore version of SRT algorithm
