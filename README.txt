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
I'll do a multicore version of SRT algorithm, with priority towards processor affinity over executing the shortest processes.

I will be taking into account the efficiencies of keeping a process running
on the same processor (processor affinity)

heres how the logic should go:
for each processor we have:
    if we are running a process already:
        if the process needs to request something, then make the request, block the process, and free the processor
        otherwise, if the process has finished, set the state to finished and free the processor
        otherwise, if all other processors are busy and if there are new processes or any interrupts:
            if there is a new process waiting to enter the ready list, free the processor and switch the new process to ready
            otherwise, if there is a recource interrupt relating to a process, free the processor, handle the interrupt, and switch the blocked process to ready
        otherwise:
            if any ready process has no affinity or has affinity for this processor and has a shorter execution time than the process currently running:
                start executing the shorter process and switch the previous process to ready
            otherwise, if the shortest ready process is shorter than a currently running process:
                free this processor and switch the affinity of the shortest process to the processor running the process that is longer than the shortest process
            otherwise, continue executing the current process.
    otherwise, if we are not doing anything:
        if there is a new process waiting to enter the ready list, switch the new process to ready
        otherwise, if there is a recource interrupt relating to a process, handle the interrupt and switch the blocked process to ready
        otherwise, if the shortest ready process has no affinity or has an affinity equal to that of the current processor:
                set affinity to current processor if needed, and begin executing the process
        otherwise, if the shortest ready process is shorter than a currently running process on another processor:
            switch the affinity of the shortest process to that of the processor running the process longer than the shortest process
        otherwise, if there is a ready process, switch the shortest ready process' affinity to the current processor
        otherwise, if there exists a blocked process or if there are any busy processors, take no further action
        otherwise, all processes have been finished to completion.