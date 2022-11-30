#include "ioModule.h"

void printResources(vector<Resource>& resourceVect)
{
    for (auto & Resc : resourceVect)
    {
        vector<int> waitListTemp;
        cout << "{";
        if (Resc.isAvailable()) { cout << " "; } else { cout << Resc.getProcessId(); }
        cout << "}";
        Resc.getWaitingProcesses(waitListTemp);
        if (waitListTemp.size() > 0) { cout << "<"; }
        for(unsigned int i = 0; i < waitListTemp.size(); i++)
        {
            cout << "-" << waitListTemp[i];
        }
        cout << "\t";
    }
    
}