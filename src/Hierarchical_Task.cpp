#include "Hierarchical_Task.h"


vc_utils::Hierarchical_Task::Hierarchical_Task( std::string _name, ProcessUnit_Base* _pUnit,
    const unsigned int _vertexNumber, const unsigned int _vertexColour, const sc_time_t& _latency )
    : Subject( _name ),
      m_ProcessUnit( _pUnit ),
      m_vertexNumber( _vertexNumber ),
      m_vertexColor( _vertexColour ),
      m_vertexLatency( _latency )
{
}
