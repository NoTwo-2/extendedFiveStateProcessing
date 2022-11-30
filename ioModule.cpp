#include "ioModule.h"

void printResources(vector<Resource>& resourceVect)
{
    vector<int> waitListTemp;
    for (auto & Resc : resourceVect)
    {
        cout << "{";
        if (Resc.isAvailable()) { cout << " "; } else { cout << Resc.getProcessId(); }
        cout << "}<-";
        Resc.getWaitingProcesses(waitListTemp);
        for(unsigned int i = 0; i < waitListTemp.size(); i++)
        {
            cout << waitListTemp[i] << "-";
        }
        cout << "\t";
    }
    
}