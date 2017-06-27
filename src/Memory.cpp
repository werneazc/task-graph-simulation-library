#include "Memory.h"


void vc_utils::Memory::notifyObservers(unsigned int _outValueId)
{
    sc_assert(m_valueInfoMap.count(_outValueId));

    for (auto obs : this->m_observerVec)
    {
        if (obs.second == _outValueId)
        {
            obs.first->notify(sc_core::SC_ZERO_TIME,
                m_valueInfoMap[_outValueId].first, m_valueInfoMap[_outValueId].second);
        }

    }
}

void vc_utils::Memory::notifyForGeneratedOutPix(void)
{
    using namespace std;

    while (true)
    {
        // And list because their is no possible way to ask an event
        // if it is notified or not
        // I'd like to use a or list but then I need another way to get information
        // which event has been changed
        sc_core::wait(m_outPixEvAndList);

        // the simple memory generates an output at the std output:
        cout << "@ " << setw(5) << sc_core::sc_time_stamp();
        cout << " | delta cycle: " << setw(5) << sc_core::sc_delta_count() << std::endl;
        cout << "Written values from ";
        this->dump();
        cout << endl;
        cout << "values: ";
        dumpOutPixel();

    }

    return;
}

void vc_utils::Memory::NotifyAllCurrentValues(void)
{
    for (auto key : m_valueInfoMap)
        this->notifyObservers(key.first);

    return;
}

void vc_utils::Memory::dumpOutPixel(std::ostream& _os /*= ::std::cout*/) const
{
    for (auto& out : m_outputValueMap)
    {
        _os << out.second.get()->m_name << ": " <<"pixel value " << out.first << ":\t";

        switch (out.second->m_dataType)
        {
        case TYPE::CHAR:
        {
            auto valuePtr = static_cast<MemoryValue<char>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::SIGNED_CHAR:
        {
            auto valuePtr = static_cast<MemoryValue<signed char>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::UNSIGNED_CHAR:
        {
            auto valuePtr = static_cast<MemoryValue<unsigned char>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::SHORT:
        {
            auto valuePtr = static_cast<MemoryValue<short>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::UNSIGNED_SHORT:
        {
            auto valuePtr = static_cast<MemoryValue<unsigned short>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::INT:
        {
            auto valuePtr = static_cast<MemoryValue<int>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::UNSIGNED_INT:
        {
            auto valuePtr = static_cast<MemoryValue<unsigned int>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::LONG:
        {
            auto valuePtr = static_cast<MemoryValue<long>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::UNSIGNED_LONG:
        {
            auto valuePtr = static_cast<MemoryValue<unsigned long>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::LONG_LONG:
        {
            auto valuePtr = static_cast<MemoryValue<long long>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::UNSIGNED_LONG_LONG:
        {
            auto valuePtr = static_cast<MemoryValue<unsigned long long>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::DOUBLE:
        {
            auto valuePtr = static_cast<MemoryValue<double>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::LONG_DOUBLE:
        {
            auto valuePtr = static_cast<MemoryValue<long double>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        case TYPE::FLOAT:
        {
            auto valuePtr = static_cast<MemoryValue<float>*>(m_outputValueMap.at(out.second->m_valueId).get());
            _os << valuePtr->m_value << std::endl;
            break;
        }
        default:
            SC_REPORT_ERROR(this->getName_Cstr(), "no valid data type found");
            break;
        };
    }

    return;
}
