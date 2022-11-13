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

    // single thread multicore processor (4 cores)
    // they're either processing something or they're not
    bitset<NUM_OF_CORES> processorsAvailable;
    processorsAvailable.set();
    
    // the process processor at index (0-3) is currently running
    // NULL if there is no process running
    Process * runningProcess[NUM_OF_CORES] = { NULL };

    // this is true when all processes are completed
    bool processesFinished = false;

    // vector of processes, processes will appear here when they are created by
    // the ProcessMgmt object (in other words, automatically at the appropriate time)
    list<Process> processList;
    
    // this will orchestrate process creation in our system, it will add processes to 
    // processList when they are created and ready to be run/managed
    ProcessManagement processMgmt(processList);

    // this is where interrupts will appear when the ioModule detects that an IO operation is complete
    list<IOInterrupt> interrupts;   

    // this manages io operations and will raise interrupts to signal io completion
    IOModule ioModule(interrupts);  

    // Do not touch
    long time = 1;
    long sleepDuration = 50;
    string file;
    stringstream ss;
    enum stepActionEnum {noAct, admitNewProc, handleInterrupt, beginRun, continueRun, ioRequest, complete} stepAction;

    // Do not touch
    switch(argc)
    {
        case 1:
            file = "./procList.txt";  // default input file
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
    while(processMgmt.moreProcessesComing() || !processesFinished)
    {
        //Update our current time step
        ++time;

        //let new processes in if there are any
        processMgmt.activateProcesses(time);
        // processList.sort(procIDComp); 

        //update the status for any active IO requests
        ioModule.ioProcessing(time);

        //If the processor is tied up running a process, then continue running it until it is done or blocks
        //   note: be sure to check for things that should happen as the process continues to run (io, completion...)
        //If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
        // - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
        // - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
        // - start processing a ready process if there are any ready

        for (int p = 0; p < NUM_OF_CORES; p++)
        {
            //init the stepAction, update below
            stepAction = noAct;

            //   <your code here> 
            // if running a process, check if we can keep running the process on this processor
            if (!processorsAvailable[p])
            {
                if (runningProcess[p])
                {
                    // if the process has an IO request at this time, submit an IO request, delete that ioEvent, and set the state to blocked
                    // cout << "IO time: " << runningProcess->ioEvents.front().time << ", Process time: " << runningProcess->processorTime << endl;
                    if (runningProcess[p]->ioEvents.front().time == runningProcess[p]->processorTime)
                    {
                        processorsAvailable[p] = true;
                        ioModule.submitIORequest(time, runningProcess[p]->ioEvents.front(), *runningProcess[p]);
                        runningProcess[p]->ioEvents.pop_front();
                        runningProcess[p]->state = blocked;
                        runningProcess[p] = NULL;
                        stepAction = ioRequest;
                    }
                    // if the process has run for the required time, free the processor and set the done time and the state to done
                    else if (runningProcess[p]->processorTime == runningProcess[p]->reqProcessorTime)
                    {
                        processorsAvailable[p] = true;
                        runningProcess[p]->state = done;
                        runningProcess[p]->doneTime = time;
                        runningProcess[p] = NULL;
                        stepAction = complete;
                    }
                    // otherwise, continue running the program. increment its processorTime.
                    else
                    {
                        runningProcess[p]->processorTime++;
                        stepAction = continueRun;
                    }
                    
                }
                else
                {
                    cerr << "Error, unavailable processor, but no running process" << endl;
                }
                
            }
            // if the processor is available and there are other processes that are ready to be run
            // TODO: this program only check for new processes and interrupts when the processor is free. This should not be the case in a STR algorithm
            // move some of this implementation to if the processor is running, basically implement the algorithm for this todo
            else if (!processList.empty())
            {
                Process * newProcess = NULL;
                unsigned int interruptProcessID;
                Process * interruptProcess = NULL;
                Process * blockedProcess = NULL;
                Process * readyProcess = NULL;
                list<Process>::iterator processListIterator = processList.begin();

                // get the ID of any interrupts
                if (!interrupts.empty())
                {
                    interruptProcessID = interrupts.front().procID;
                }
                else
                {
                    interruptProcessID = -1;
                }
                
                // iterate through to find the first new process, the blocked process that matches the interrupt, and the first ready process
                do
                {
                    Process * tempProcessPointer = &(*processListIterator);

                    if (!newProcess && processListIterator->state == newArrival)
                    {
                        newProcess = tempProcessPointer;
                    }
                    if (!blockedProcess && processListIterator->state == blocked)
                    {
                        blockedProcess = tempProcessPointer;
                    }
                    if (!interruptProcess && processListIterator->id == interruptProcessID && processListIterator->state == blocked)
                    {
                        interruptProcess = tempProcessPointer;
                    }
                    if (!readyProcess && processListIterator->state == ready)
                    {
                        readyProcess = tempProcessPointer;
                    }

                } while ((!newProcess || !interruptProcess || !readyProcess || !blockedProcess) && processListIterator++ != processList.end());

                // cout << "DABS IN SWAHILI" << endl;
                // if we found a new arrival, move this process to the ready state
                if (newProcess)
                {
                    newProcess->state = ready;
                    stepAction = admitNewProc;
                }
                // otherwise, if we have an interupt, remove the interrupt from the interrupt list and move the process to the ready state 
                else if (interruptProcess)
                {
                    interrupts.pop_front();
                    interruptProcess->state = ready;
                    stepAction = handleInterrupt;
                }
                // otherwise, if we have a ready process we will move that process to the runnning state as well as incrementing the processorTime
                else if (readyProcess)
                {
                    processorsAvailable[p] = false;
                    readyProcess->state = processing;
                    readyProcess->processorTime++;
                    runningProcess[p] = readyProcess;
                    stepAction = beginRun;
                }
                else if (blockedProcess)
                {
                    stepAction = noAct;
                }
                else
                {
                    processesFinished = true;
                }
            }
        }
        
        // TODO: revamp the output to better suit a multicore design
        if (!processesFinished)
        {
            // Leave the below alone (at least for final submission, we are counting on the output being in expected format)
            cout << setw(5) << time << "\t"; 
            
            switch(stepAction)
            {
                case admitNewProc:
                cout << "[  admit]\t";
                break;
                case handleInterrupt:
                cout << "[ inrtpt]\t";
                break;
                case beginRun:
                cout << "[  begin]\t";
                break;
                case continueRun:
                cout << "[contRun]\t";
                break;
                case ioRequest:
                cout << "[  ioReq]\t";
                break;
                case complete:
                cout << "[ finish]\t";
                break;
                case noAct:
                cout << "[*noAct*]\t";
                break;
            }

            // You may wish to use a second vector of processes (you don't need to, but you can)
            printProcessStates(processList); // change processList to another vector of processes if desired

            this_thread::sleep_for(chrono::milliseconds(sleepDuration));
        }
        
    }

    return 0;
}


