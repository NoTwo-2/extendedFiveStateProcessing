#include "process.h"
#include "ioModule.h"
#include "processMgmt.h"

#include <chrono> // for sleep
#include <thread> // for sleep
#include <bitset> // for multicore

inline bool procIDComp(const Process& p1, const Process& p2)
{
    return p1.id < p2.id;
}

int main(int argc, char* argv[])
{
    // number of cores
    const int NUM_OF_CORES = 4;
    // number of resources
    const int NUM_OF_RESOURCES = 4;

    // single thread multicore processor (4 cores)
    // they're either processing something or they're not
    bitset<NUM_OF_CORES> processorsAvailable;
    processorsAvailable.set();
    
    // the process processor at index (0-3) is currently running
    // NULL if there is no process running
    Process * runningProcess[NUM_OF_CORES] = { NULL };

    // these should all be set when all processes have been completed
    bitset<NUM_OF_CORES> coresReturnFinished;
    coresReturnFinished.reset();

    // vector of processes, processes will appear here when they are created by
    // the ProcessMgmt object (in other words, automatically at the appropriate time)
    list<Process> processList;
    
    // this will orchestrate process creation in our system, it will add processes to 
    // processList when they are created and ready to be run/managed
    ProcessManagement processMgmt(processList);

    // this holds the pointers to all resources which manage io operations and will raise interrupts to signal io completion
    vector<Resource> resources;

    // populate the resource vector
    for (int i = 0; i < NUM_OF_RESOURCES; i++)
    {
        resources.push_back(Resource(i));
    }

    // Do not touch
    long time = 1;
    long sleepDuration = 50;
    string file;
    stringstream ss;
    enum stepActionEnum {noAct, admitNewProc, handleInterrupt, beginRun, continueRun, resourceRequest, complete, swapAffinity};
    stepActionEnum stepActions[NUM_OF_CORES];
    // Do not touch
    switch(argc)
    {
        case 1:
            file = "./procList1.txt";  // default input file
            break;
        case 2:
            file = argv[1];         // file given from command line
            break;
        case 3:
            file = argv[1];         // file given
            ss.str(argv[2]);        // sleep duration given
            ss >> sleepDuration;
            break;
        default:
            cerr << "incorrect number of command line arguments" << endl;
            cout << "usage: " << argv[0] << " [file] [sleepDuration]" << endl;
            return 1;
            break;
    }

    processMgmt.readProcessFile(file);

    time = 0;

    //keep running the loop until all processes have been added and have run to completion
    while(processMgmt.moreProcessesComing() || !coresReturnFinished.all())
    {
        // reset this on every clock tick
        coresReturnFinished.reset();
        //Update our current time step
        ++time;

        //let new processes in if there are any
        processMgmt.activateProcesses(time);
        // processList.sort(procIDComp); 

        // update the status for any active IO requests
        for (int i = 0; i < NUM_OF_RESOURCES; i++)
        {
            resources[i].ioProcessing();
        }

        //If the processor is tied up running a process, then continue running it until it is done or blocks
        //   note: be sure to check for things that should happen as the process continues to run (io, completion...)
        //If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
        // - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
        // - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
        // - start processing a ready process if there are any ready

        for (int p = 0; p < NUM_OF_CORES; p++)
        {
            //init the stepAction, update below
            stepActions[p] = noAct;

            // pointers for storing the location of processes that are best sutied for each category
            // holds first new process in process list
            Process * newProcess = NULL;
            // holds the first blocked process in the process list
            Process * blockedProcess = NULL;
            // holds the process with the shortest time remaining in the process list
            Process * shortest = NULL;
            // holds the shortest process that has an affinity equal to that of the current processor
            Process * shortestWithAffinity = NULL;
            list<Process>::iterator it = processList.begin();
            
            // iterate through to find the first new process, the blocked process that matches the interrupt, and the first ready process
            do
            {
                // we do this to change the iterator into a regular pointer
                Process * tempProcessPointer = &(*it);
                // difference variables, finding remaining time for processes
                long sWARemainingTime = 0;
                long shortestRemainingTime = 0;
                if (shortestWithAffinity)
                {
                    sWARemainingTime = shortestWithAffinity->reqProcessorTime - shortestWithAffinity->processorTime;
                }
                if (shortest)
                {
                    shortestRemainingTime = shortest->reqProcessorTime - shortest->processorTime;
                }
                long itRemainingTime = it->reqProcessorTime - it->processorTime;

                if (!newProcess && it->state == newArrival)
                {
                    newProcess = tempProcessPointer;
                }
                if (!blockedProcess && it->state == blocked)
                {
                    blockedProcess = tempProcessPointer;
                }
                if ((!shortestWithAffinity || sWARemainingTime > itRemainingTime) && it->state == ready && (it->affinity == p || it->affinity == -1))
                {
                    shortestWithAffinity = tempProcessPointer;
                }
                if ((!shortest || shortestRemainingTime > itRemainingTime) && it->state == ready)
                {
                    shortest = tempProcessPointer;
                }
                // cout << (it->id) << endl;
            } while (++it != processList.end());

            // if (shortest && shortestWithAffinity)
            // {
            //     cout << shortest->id << " " << shortestWithAffinity->id << endl;
            // }


            // if we are running a process, check if we can keep running the process on this processor, or if we need to interrupt execution for any reason
            if (!processorsAvailable[p])
            {
                if (runningProcess[p])
                {
                    // TODO: DEADLOCK PREVENTION: GET ON IT!!
                    // processes need to keep track of the resource they are requesting, resources need to keep track of the processes that are requesting them
                    // if the process has an IO request at this time, submit a resource request, delete that ioEvent, and block the process if it needs to be blocked
                    // cout << "IO time: " << runningProcess[p]->ioEvents.front().time << ", Process time: " << runningProcess[p]->processorTime << endl;
                    if (runningProcess[p]->ioEvents.front().time == runningProcess[p]->processorTime)
                    {
                        while (runningProcess[p]->ioEvents.front().time == runningProcess[p]->processorTime)
                        {
                            if (!resources[(runningProcess[p]->ioEvents.front()).resourceId].submitRequest(runningProcess[p]->ioEvents.front(), *runningProcess[p]))
                            {
                                processorsAvailable[p] = true;
                                runningProcess[p]->state = blocked;
                            }
                            runningProcess[p]->ioEvents.pop_front();
                        }
                        stepActions[p] = resourceRequest;
                    }
                    // if the process has run for the required time, free the processor and set the done time and the state to done
                    else if (runningProcess[p]->processorTime == runningProcess[p]->reqProcessorTime)
                    {
                        processorsAvailable[p] = true;
                        runningProcess[p]->state = done;
                        runningProcess[p]->doneTime = time;
                        runningProcess[p] = NULL;
                        stepActions[p] = complete;
                    }
                    // otherwise, if all other processors are busy, we need to check if we can admit any new or blocked processes since SRT is preemptive
                    else if (static_cast<int>(processorsAvailable.count()) == p && newProcess)
                    {
                        processorsAvailable[p] = true;
                        runningProcess[p]->state = ready;
                        runningProcess[p] = NULL;
                        // if we found a new arrival, move this process to the ready state
                        if (newProcess)
                        {
                            newProcess->state = ready;
                            stepActions[p] = admitNewProc;
                        }
                        else
                        {
                            cerr << "Error, terrible logic in main.cpp" << endl;
                        }
                        
                    }
                    // otherwise, we should continue executing our process
                    else
                    {
                        runningProcess[p]->processorTime++;
                        stepActions[p] = continueRun;
                    }
                    
                }
                else
                {
                    cerr << "Error, unavailable processor, but no running process" << endl;
                }
                
            }
            // if the processor is available and there are other processes that are ready to be run
            else if (!processList.empty())
            {
                // if we found a new arrival, move this process to the ready state
                if (newProcess)
                {
                    newProcess->state = ready;
                    stepActions[p] = admitNewProc;
                }
                // otherwise, if we have a ready process we will move that process to the runnning state as well as incrementing the processorTime
                else if (shortestWithAffinity)
                {
                    processorsAvailable[p] = false;
                    shortestWithAffinity->affinity = p;
                    shortestWithAffinity->state = processing;
                    shortestWithAffinity->processorTime++;
                    runningProcess[p] = shortestWithAffinity;
                    stepActions[p] = beginRun;
                }
                // if there is still a ready process, but has an affinity of a processor that is busy, we need to take time to swap this affinity
                // this is to simulate the inefficiency of running processes on different processors during their lifetimes
                else if (shortest && !processorsAvailable[shortest->affinity])
                {
                    shortest->affinity = p;
                    stepActions[p] = swapAffinity;
                }
                // otherwise, take no action
                else if (blockedProcess || !processorsAvailable.all())
                {
                    stepActions[p] = noAct;
                }
                else
                {
                    coresReturnFinished[p] = true;
                }
            }
        }

        // Leave the below alone (at least for final submission, we are counting on the output being in expected format)
        cout << setw(5) << time << "\t"; 
        
        for (int p = 0; p < NUM_OF_CORES; p++)
        {
            char processorName = '0' + p;
            switch(stepActions[p])
            {
                case admitNewProc:
                cout << processorName << ": [  admit] ";
                break;
                case handleInterrupt:
                cout << processorName << ": [ inrtpt] ";
                break;
                case beginRun:
                cout << processorName << ": [  begin] ";
                break;
                case continueRun:
                cout << processorName << ": [contRun] ";
                break;
                case resourceRequest:
                cout << processorName << ": [rescReq] ";
                break;
                case complete:
                cout << processorName << ": [ finish] ";
                break;
                case noAct:
                cout << processorName << ": [*noAct*] ";
                break;
                case swapAffinity:
                cout << processorName << ": [ swpAff] ";
                break;
            }
        }
        cout << "\t";
        // You may wish to use a second vector of processes (you don't need to, but you can)
        printProcessStates(processList); // change processList to another vector of processes if desired
        cout << "\t";
        printResources(resources);

        cout << endl;
        this_thread::sleep_for(chrono::milliseconds(sleepDuration));  
    }

    return 0;
}


