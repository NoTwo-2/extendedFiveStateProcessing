#ifndef FIVE_STATE_IO_H
#define FIVE_STATE_IO_H

#include<vector>
using namespace std;

#include "process.h"

struct ResourceRequest
{
    ResourceRequest() : ioEventID(99999), procID(99999), requiredTime(0), elapsedTime(0) {};
    ResourceRequest(const unsigned int& eId, const unsigned int& pId, const unsigned int& reqT) : 
        ioEventID(eId), procID(pId), requiredTime(reqT), elapsedTime(0) {};

    unsigned int ioEventID;
    unsigned int procID;

    long requiredTime;  // time this particular request needs to be running with the process
    long elapsedTime;   // time this request has been served
};

class Resource
{
    public:
        Resource(vector<Process*>& allocatedProcessList, const unsigned int& rId) : m_resourceId(rId), m_allocatedProcesses(allocatedProcessList) {};

        inline void ioProcessing()
        {
            // ( ( time_remaining, IOInterrupt ), Process )
            // cout << curTimeStep << " " << m_pending.first << endl;
            // if the process isnt blocked, increment the resources time spent
            if (m_pending.second && m_pending.second->state == processing) { m_pending.first.elapsedTime++; }
            
            // if the resources time spent is equal to the required time, we want to free this resource
            if (m_pending.second && m_pending.second->state == processing && m_pending.first.elapsedTime == m_pending.first.requiredTime)
            {
                unallocate();
            }
        }

        bool submitRequest(const unsigned int& eventId, const unsigned int& duration, Process* proc)
        {
            bool success;
            if (m_available)
            {
                m_pending = make_pair(ResourceRequest(eventId, proc->id, duration), proc);
                proc->otherResourcesIds.push_back(m_resourceId);
                m_available = false;

                success = true;
            }
            else
            {
                m_waitingList.push_back(make_pair(ResourceRequest(eventId, proc->id, duration), proc));

                success = false;
            }
            return success;
        }

        void unallocateResource()
        {
            if (!m_available)
            {
                m_waitingList.push_back(m_pending);
                
                unallocate();
            }
        }

        inline bool isAvailable() { return m_available; }
        inline int getProcessId() { return m_pending.first.procID; }
        inline void getWaitingProcesses(vector<unsigned int>& waiting)
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

        vector<Process*>& m_allocatedProcesses;

        void unallocate()
        {
            vector<int>::iterator rescPos = m_pending.second->otherResourcesIds.begin();
            // find the resource in the otherResource vector in the pcb
            while (rescPos != m_pending.second->otherResourcesIds.end() && static_cast<unsigned int>(*rescPos) != m_resourceId) { rescPos++; }
            // remove the resource from the list of resources the process has allocated to it
            m_pending.second->otherResourcesIds.erase(rescPos);

            // if there are other processes waiting, allocate the resource to the next process in the waitlist
            if (!m_waitingList.empty())
            {

                // allocate the next process in the waiting list
                m_pending = m_waitingList.front();
                m_allocatedProcesses.push_back(m_waitingList.front().second);
                m_waitingList.front().second->otherResourcesIds.push_back(m_resourceId);
                m_waitingList.pop_front();
            }
            // otherwise, this resource is free
            else
            {
                m_available = true;
            }
        }
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