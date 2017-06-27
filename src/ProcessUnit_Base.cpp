#include "ProcessUnit_Base.h"

namespace vc_utils
{


    ProcessUnit_Base::ProcessUnit_Base(name_t _name, unsigned int _unitId) :
        sc_core::sc_module(_name), m_unitId(_unitId)
    {
        m_vertices.clear();
    }

    void ProcessUnit_Base::isCoreUsed(event_t* _event)
    {
        if (m_coreUsed)
            m_processWaitingQueue.push(_event);
        else
        {
            m_coreUsed = true;
            _event->notify(sc_core::SC_ZERO_TIME);
        }
    }

    void ProcessUnit_Base::freeUsedCore(const sc_time_t& _latency)
    {
        if (!m_processWaitingQueue.empty())
        {
            m_processWaitingQueue.front()->notify(_latency);
            m_processWaitingQueue.pop();
        }
        else
        {
            m_coreUsed = false;
            sc_core::wait(_latency);
        }
    }

    const char* ProcessUnit_Base::kind() const
    {
        return "Process Unit Base";
    }

    void ProcessUnit_Base::print(::std::ostream& os /*= ::std::cout*/) const
    {
        os << this->name();
    }

    void ProcessUnit_Base::dump(::std::ostream& os /*= ::std::cout*/) const
    {
        os << this->name() << ", " << this->kind();
    }

}