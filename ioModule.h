#ifndef FIVE_STATE_IO_H
#define FIVE_STATE_IO_H

#include<vector>
using namespace std;

#include "process.h"

struct ResourceRequest
{
    ResourceRequest() : ioEventID(99999), procID(99999), resourceID(99999), requiredTime(0), elapsedTime(0) {};
    ResourceRequest(const unsigned int& eId, const unsigned int& pId, const unsigned int& rId, const unsigned int& reqT) : 
        ioEventID(eId), procID(pId), resourceID(rId), requiredTime(reqT), elapsedTime(0) {};

    unsigned int ioEventID;
    unsigned int procID;
    unsigned int resourceID;

    long requiredTime;  // time this particular request needs to be running with the process
    long elapsedTime;   // time this request has been served
};

class Resource
{
    public:
        Resource(const unsigned int& rId) : m_resourceId(rId) {};

        inline void ioProcessing()
        {
            // ( ( time_remaining, IOInterrupt ), Process )
            // cout << curTimeStep << " " << m_pending.first << endl;
            if (m_pending.second && m_pending.second->state == processing) { m_pending.first.elapsedTime++; }
            if (m_pending.second && m_pending.first.elapsedTime == m_pending.first.requiredTime)
            {
                if (!m_waitingList.empty())
                {
                    cout << "waitlist not empty, allocating to next in line" << endl;
                    m_pending = m_waitingList.front();
                    // if (m_waitingList.front().second->)
                    m_waitingList.front().second->state = ready;
                    m_waitingList.front().second->otherResourcesIds.push_back(m_resourceId);
                    m_waitingList.pop_front();
                }
                else
                {
                    cout << "waitlist empty, resource available" << endl;
                    m_available = true;
                }
            }
        }

        bool submitRequest(IOEvent& ioEvent, Process& proc)
        {
            bool success;
            if (m_available)
            {
                cout << "resource available, processing request..." << endl;
                m_pending = make_pair(ResourceRequest(ioEvent.id, proc.id, m_resourceId, ioEvent.dur), &proc);
                proc.otherResourcesIds.push_back(m_resourceId);
                m_available = false;

                success = true;
            }
            else
            {
                cout << "resource is unavailable, adding to wait list" << endl;
                m_waitingList.push_back(make_pair(ResourceRequest(ioEvent.id, proc.id, m_resourceId, ioEvent.dur), &proc));

                success = false;
            }
            return success;
        }

        inline bool isAvailable() { return m_available; }
        inline int getProcessId() { return m_pending.first.procID; }
        inline void getWaitingProcesses(vector<int>& waiting)
        {
            list<pair<ResourceRequest, Process*>>::iterator it;
            for (it = m_waitingList.begin(); it != m_waitingList.end(); ++it)
            {
                waiting.push_back(it->first.procID);
            }
        }

    private:
        pair<ResourceRequest, Process*> m_pending;   // the io request to this vector that needs to be serviced

        bool m_available = true;
        list<pair<ResourceRequest, Process*>> m_waitingList;
        unsigned int m_resourceId;
};

void printResources(vector<Resource>& resourceVect);

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