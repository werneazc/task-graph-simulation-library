//! \file Task_Base.h
//! \brief Properties of a task module

#ifndef TASK_BASE_H_
#define TASK_BASE_H_

#include "Typedefinitions.h"
#include "Subject.h"
#include "ObserverManager.h"

namespace vc_utils
{

    /*!
     * \class Task_Base
     *
     * \brief Properties of a task module
     *
     * \author Andre Werner
     * \date Juno 2015
     */
    class Task_Base : public Subject
    {

    public:
        /************************************************************************/
        // Observers
        //! \var inputObs
        //! \brief short description
        /************************************************************************/
        ObserverManager<Observer> inputObs;

    public:
        virtual ~Task_Base( ) = default; //!< \brief destructor

        //! \brief notify children observers for output values
        virtual void notifyObservers( unsigned int _outputId ) = 0;

        //! \brief execute works on local members at specific task
        virtual void execute( void ) = 0;

        // virtual unsigned int executeDebug( void ) = 0; !not implemented yet!

        //! \brief return vertex number of a task graph vertex
        unsigned int getVertexNumber( void ) const { return m_vertexNumber; }

        //! \brief return vertex color of a task graph vertex
        unsigned int getVertexColor( void ) const { return m_vertexColor; }

        //! \brief return task graph vertex latency (process costs)
        const sc_time_t& getVertexLatency( void ) const { return m_vertexLatency; }

        //! \brief set vertex number of a task graph vertex
        void setVertexNumber( const unsigned int _number ) { m_vertexNumber = _number; }

          //! \brief set vertex color of a task graph vertex
        void setVertexColor( const unsigned int _color ) { m_vertexColor = _color; }

        //! \brief set task graph vertex latency (process costs)
        void setVertexLatency( const double& _latency, unit_t _unit = sc_core::SC_NS )
        {
            m_vertexLatency = sc_time_t( sc_dt::sc_abs< double >( _latency ), _unit );
        }

        //! \brief set task graph vertex latency (process costs)
        void setVertexLatency( const sc_time_t& _latency ) { m_vertexLatency = _latency; }

        //! \brief for using in ordered maps
        bool operator<( const Task_Base& _rhs )
        {
            if ( this->get_subjectID( ) < _rhs.get_subjectID( ) )
                return true;
            else
                return false;
        }

        //! \brief return type of vertex class as string
        std::string getClassType( void ) const { return m_classtype; }

        //! \brief set class type of vertex
        void setClassType( const std::string _type ) { m_classtype = _type; }

    protected:
        /************************************************************************/
        /* constructor                                                          */
        /************************************************************************/

        //! \brief constructor with systemC time object
        explicit Task_Base( std::string _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, double _latency = 0.0, unit_t _unit = sc_core::SC_NS )
            : Subject( _name ),
              m_vertexNumber( _vertexNumber ),
              m_vertexColor( _vertexColor ),
              m_vertexLatency( sc_time_t( _latency, _unit ) ),
              m_classtype( "" )
        {
        }

        //! \brief constructor
        explicit Task_Base( std::string _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, const sc_time_t& _latency = sc_core::SC_ZERO_TIME )
            : Subject( _name ),
              m_vertexNumber( _vertexNumber ),
              m_vertexColor( _vertexColor ),
              m_vertexLatency( _latency ),
              m_classtype( "" )
        {
        }

    private:
        //! \var m_vertexNumber
        //! \brief task graph vertex number
        unsigned int m_vertexNumber;
        //! \var m_vertexColor
        //! \brief task graph vertex clustering color
        unsigned int m_vertexColor;
        //! \var m_vertexLatency
        //! \brief task graph vertex process costs
        sc_time_t m_vertexLatency;
        //! \var classtype
        //! \brief save vertex class type
        std::string m_classtype;
        //!< string of class type including template parameters

    private:
        Task_Base( ) = delete;                                  //!< \brief forbidden
        Task_Base( const Task_Base& _source ) = delete;         //!< \brief forbidden
        Task_Base( Task_Base&& _source ) = delete;              //!< \brief forbidden
        Task_Base& operator=( const Task_Base& _rhs ) = delete; //!< \brief forbidden
        Task_Base& operator=( Task_Base&& _rhs ) = delete;      //!< \brief forbidden
    };
}


#endif
