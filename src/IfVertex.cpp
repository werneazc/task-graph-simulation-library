//! \file IfVertex.cpp
//! \brief If vertex implementation file.

#include "IfVertex.h"


namespace vc_utils
{

    /************************************************************************/
    // then path method implementations:
    /************************************************************************/
    // constructor:
    IfVertex::ThenPath::ThenPath(IfVertex* _parentIf, std::string _name)
        : Subject(_name), m_parentIf(_parentIf)
    {
        m_vertices.clear();
    }

    // notify observers:
    void IfVertex::ThenPath::notifyObservers(unsigned int _outValueId)
    {

        for (auto obs : this->m_observerVec)
        {
            // get data to send
            auto data = m_parentIf->m_ifBeginDataVec[_outValueId].first;
            auto length = m_parentIf->m_ifBeginDataVec[_outValueId].second;

            // search for every Observer that is sensitive for value changes at
            // _outValueId
            if (obs.second == _outValueId)
            {
                obs.first->notify(sc_core::SC_ZERO_TIME, data, length);
            }
        }
    }

    /************************************************************************/
    // else path method implementations:
    /************************************************************************/
    // constructor:
    IfVertex::ElsePath::ElsePath(IfVertex* _parentIf, std::string _name)
        : Subject(_name), m_parentIf(_parentIf)
    {
        m_vertices.clear();
    }

    // notify observers:
    void IfVertex::ElsePath::notifyObservers(unsigned int _outValueId)
    {

        for (auto obs : this->m_observerVec)
        {
            // get data to send
            auto data = m_parentIf->m_ifBeginDataVec[_outValueId].first;
            auto length = m_parentIf->m_ifBeginDataVec[_outValueId].second;

            // search for every Observer that is sensitive for value changes at
            // _outValueId
            if (obs.second == _outValueId)
            {
                obs.first->notify(sc_core::SC_ZERO_TIME, data, length);
            }
        }
    }

    /************************************************************************/
    // else path method implementations:
    /************************************************************************/
    //constructor
    IfVertex::IfVertex(name_t _name, ProcessUnit_Base* _unit, unsigned int _vertexColor,
        unsigned int _vertexNumber, sc_time_t _latency, unsigned int _numOfInEdges,
        Subject* const _condition)
        : sc_core::sc_module(_name),
        Hierarchical_Task(std::string(_name), _unit, _vertexNumber, _vertexColor, _latency),
        m_conditionEv((std::string(_name) + "_conditionEv").c_str()),
        m_elsePath(this, std::string(_name) + "_elsePath"),
        m_thenPath(this, std::string(_name) + "_thenPath"),
        m_ifBeginDataVec(_numOfInEdges),
        m_ifEndDataVec(_numOfInEdges)
    {
        // register observer for condition automatically:
        m_conditionObs.addObserver(
            &m_conditionEv, reinterpret_cast<dataPtr_t>(&m_condition), sizeof(bool));
        _condition->registerObserver(m_conditionObs.getObserver(0), 0);

        // generate synchronization events and observer for if begin
        for (unsigned int i = 0; i < _numOfInEdges; ++i)
        {
            m_ifBeginEvVec.emplace_back(new event_t(
                (this->getName() + "_inEdgeEv" + std::to_string(i)).c_str()));
            inputObs.addObserver(m_ifBeginEvVec[i],
                reinterpret_cast<dataPtr_t>(&m_ifBeginDataVec[i]), sizeof(dataVec_t));
        }

        // register events for condition check:
        for (auto event : m_ifBeginEvVec)
            m_ifBeginEvAndList &= *event;
        m_ifBeginEvAndList &= m_conditionEv;

        // register SystemC threads at scheduler
        SC_THREAD(conditionCheck);
        SC_THREAD(ifEndFromThenProcess);
        SC_THREAD(ifEndFromElseProcess);


        //reset node lists
        m_thenNodes.clear();
        m_elseNodes.clear();
    }

    //destructor
    IfVertex::~IfVertex()
    {
        // delete all dynamically generated events
        for (auto event : m_ifBeginEvVec)
            delete event;

        for (auto event : m_ifEndEvVec)
            delete event;
    }

    //SystemC threads

    void IfVertex::conditionCheck(void)
    {
        while (true)
        {
            sc_core::wait(m_ifBeginEvAndList);

            // If data is not change in one of the paths, the reference to this
            // value is copied for access by if-node successors.
            // If value is changed, the data reference is overwritten by observer
            // of m_ifEndObs.
            m_ifEndDataVec = m_ifBeginDataVec;

            // check which path has to be performed
            if (m_condition)
            {
                for (auto valueId : m_thenNodes)
                    m_thenPath.notifyObservers(valueId);
            }
            else
            {
                for (auto valueId : m_elseNodes)
                    m_elsePath.notifyObservers(valueId);
            }
        }
    }

    void IfVertex::ifEndFromThenProcess(void)
    {
        while (true)
        {
            sc_core::wait(m_ifEndFromThenEvAndList);

            for (auto valueId = 0u; valueId < m_ifBeginDataVec.size(); ++valueId)
                this->notifyObservers(valueId);
        }
    }

    void IfVertex::ifEndFromElseProcess(void)
    {
        while (true)
        {
            sc_core::wait(m_ifEndFromElseEvAndList);

            for (auto valueId = 0u; valueId < m_ifBeginDataVec.size(); ++valueId)
                this->notifyObservers(valueId);
        }
    }

    //structure building methods:

    void IfVertex::registerThenOutDependency(unsigned int _subNodeId, unsigned int _inEdgeId, unsigned int _valId)
    {
        //check that Subject is a vertex of then path
        if (!m_thenPath.m_vertices.count(_subNodeId))
            SC_REPORT_ERROR(
            this->name(), "no valid identification number for node in then path");

        //generate new event for if then path end synchronization
        std::string name = this->getName() + "_outEdgeEv" + std::to_string(m_ifEndEvVec.size());
        m_ifEndEvVec.emplace_back(new event_t(name.c_str()));
        m_ifEndFromThenEvAndList &= *m_ifEndEvVec.back();

        //generate new Observer for that value;
        auto currentObsId = m_ifEndObs.addObserver(m_ifEndEvVec.back(), reinterpret_cast<dataPtr_t>(&m_ifEndDataVec[_inEdgeId]), sizeof(dataVec_t));

        //register Observer at then path node
        auto subNode = m_thenPath.m_vertices[_subNodeId];
        subNode->registerObserver(m_ifEndObs.getObserver(currentObsId), _valId);
    }

    void IfVertex::registerElseOutDependency(unsigned int _subNodeId, unsigned int _inEdgeId, unsigned int _valId)
    {
        //check that Subject is a vertex of then path
        if (!m_elsePath.m_vertices.count(_subNodeId))
            SC_REPORT_ERROR(
            this->name(), "no valid identification number for node in else path");

        //generate new event for if then path end synchronization
        std::string name = this->getName() + "_outEdgeEv" + std::to_string(m_ifEndEvVec.size());
        m_ifEndEvVec.emplace_back(new event_t(name.c_str()));
        m_ifEndFromElseEvAndList &= *m_ifEndEvVec.back();

        //generate new Observer for that value;
        auto currentObsId = m_ifEndObs.addObserver(m_ifEndEvVec.back(), reinterpret_cast<dataPtr_t>(&m_ifEndDataVec[_inEdgeId]), sizeof(dataVec_t));

        //register Observer at else path node
        auto subNode = m_elsePath.m_vertices[_subNodeId];
        subNode->registerObserver(m_ifEndObs.getObserver(currentObsId), _valId);
    }

    Subject* const IfVertex::getThenPathNode(unsigned int _vertexId)
    {
        //check if 
        if (!m_thenPath.m_vertices.count(_vertexId))
            SC_REPORT_ERROR(this->name(),
            "vertex identification number index no node in then path");

        return m_thenPath.m_vertices[_vertexId];
    }

    Subject* const IfVertex::getElsePathNode(unsigned int _vertexId)
    {
        //check if 
        if (!m_elsePath.m_vertices.count(_vertexId))
            SC_REPORT_ERROR(this->name(),
            "vertex identification number index no node in else path");

        return m_elsePath.m_vertices[_vertexId];
    }

    //notify if successors
    void IfVertex::notifyObservers(unsigned int _outValueId)
    {
        // get data to send
        auto data = m_ifEndDataVec[_outValueId].first;
        auto length = m_ifEndDataVec[_outValueId].second;

        // search for every Observer that is sensitive for value changes at _outValueId
        for (auto _obs : this->m_observerVec)
        {
            if (_obs.second == _outValueId)
            {
                _obs.first->notify(sc_core::SC_ZERO_TIME, data, length);
            }
        }
    }

}