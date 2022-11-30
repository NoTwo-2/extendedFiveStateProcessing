#ifndef FIVE_STATE_IO_H
#define FIVE_STATE_IO_H

#include<vector>
using namespace std;

#include "process.h"

struct IOInterrupt
{
    IOInterrupt() : ioEventID(99999), procID(99999), resourceID(99999) {};
    IOInterrupt(const unsigned int& eId, const unsigned int& pId, const unsigned int& rId) : ioEventID(eId), procID(pId), resourceID(rId) {};

    unsigned int ioEventID;
    unsigned int procID;
    unsigned int resourceID;
};

class Resource
{
    public:
        Resource(list<IOInterrupt>& ioIntVec, const unsigned int& rId, const unsigned long& dur) : m_intVec(ioIntVec), m_resourceId(rId), m_duration(dur) {};

        inline void ioProcessing(const int& curTimeStep)
        {
            if(curTimeStep == m_pending.first)
            {
                m_intVec.push_back(m_pending.second);
                
                m_available = true;
            }
        }

        void submitIORequest(const int& curTimeStep, IOEvent& ioEvent, Process& proc)
        {
            if (m_available)
            {
                m_pending = make_pair(curTimeStep + m_duration, IOInterrupt(ioEvent.id, proc.id, m_resourceId));
                m_available = false;
            }
            else
            {
                m_waitingList.push_back(proc);
            }
            
        }

        // inline bool isAvailable() { return m_available; }

    private:
        list<IOInterrupt>& m_intVec;        // the vector with interrupts that need to be addressed in main
        pair<int, IOInterrupt> m_pending;   // the io request to this vector that needs to be serviced

        bool m_available = false;
        list<Process> m_waitingList;
        unsigned int m_resourceId;
        unsigned long m_duration;
};

// class ResourceModule
// {
//     public:
//         ResourceModule(list<IOInterrupt>& ioIntVec, vector<Resource>& resourceVec) : m_intVec(ioIntVec), m_rescVec(resourceVec) {}

//         inline void ioProcessing(const int& curTimeStep)
//         {
//             for(int i = 0, i_end = m_pending.size(); i < i_end; ++i)
//             {
//                 if(curTimeStep == m_pending[i].first)
//                 {
//                     m_intVec.push_back(m_pending[i].second);
//                     m_pending.erase(m_pending.begin() + i);
//                     m_rescVec[m_pending[i].second.resourceID].available = true;
//                     --i;
//                     --i_end;
//                 }
//             }
//         }

//         inline void submitIORequest(const int& curTimeStep, const IOEvent& ioEvent, const Process& proc)
//         {
//             m_pending.push_back(make_pair(curTimeStep + m_rescVec[ioEvent.resourceId].duration, IOInterrupt(ioEvent.id, proc.id, ioEvent.resourceId)));
//             m_rescVec[ioEvent.resourceId].available = false;
//         }

//     private:
//         list<IOInterrupt>& m_intVec;
//         vector<Resource>& m_rescVec;
//         vector<pair<int, IOInterrupt> > m_pending;
// };

#endif