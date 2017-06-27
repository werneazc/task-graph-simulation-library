//! \file Hierarchical_Task.h
//! \brief Basic structure of a hierarchical task graph node

#ifndef HIERARCHICAL_TASK_H_
#define HIERARCHICAL_TASK_H_

#include "Typedefinitions.h"
#include "Subject.h"

namespace vc_utils
{

    /************************************************************************/
    // declarations
    /************************************************************************/
    struct ProcessUnit_Base;


    /************************************************************************/
    //! \struct Hierarchical_Task
    //!
    //! \brief Base class for hierarchical task graph nodes
    //!
    //! \details
    //! This struct contains all members and interfaces that a hierarchical
    //! node should have.
    //! Through specific requirements of hierarchy structured nodes like if,
    //! for or while, only some data members and getter methods are implemented
    //! in their common base class.
    //!
    /************************************************************************/
    struct Hierarchical_Task : public Subject
    {

    protected:
        /***************************************************************/
        // constructor:
        //!
        //! \brief    standard constructor
        //!
        //! \param [in] _name Subject name string
        //! \param [in] _pUnit pointer to process unit which implements task
        //! \param [in] _vertexNumber vertex number of task graph node
        //! \param [in] _vertexColour clustering color of vertex
        //! \param [in] _latency computation latency of task graph node
        //!
        //!
        //! \author    Andre Werner
        //! \version   2015-7-15 : initial
        /***************************************************************/
        explicit Hierarchical_Task( std::string _name, ProcessUnit_Base* _pUnit,
            const unsigned int _vertexNumber, const unsigned int _vertexColour,
            const sc_time_t& _latency );

    private:
        // forbidden constructors
        Hierarchical_Task( ) = delete; //!< \brief forbidden constructor
        Hierarchical_Task(
            const Hierarchical_Task& _source ) = delete;           //!< \brief forbidden constructor
        Hierarchical_Task( Hierarchical_Task&& _source ) = delete; //!< \brief forbidden constructor
        Hierarchical_Task& operator=(
            const Hierarchical_Task& _rhs ) = delete; //!< \brief forbidden constructor
        Hierarchical_Task& operator=(
            Hierarchical_Task&& _rhs ) = delete; //!< \brief forbidden constructor

    public:
        virtual ~Hierarchical_Task( ) = default;

    public:
        /***************************************************************/
        // notifyObservers:
        //!
        //! \brief    Observer notification interface for Subject
        //!
        //! \param [in] _outValueId value ID for notification
        //!
        //! \author    Andre Werner
        //! \version   2015-7-15 : initial
        /***************************************************************/
        virtual void notifyObservers( unsigned int _outValueId ) override = 0;

    public:
        /************************************************************************/
        // getter
        /************************************************************************/
        //! \brief return number of generated nodes inside if vertex
        inline unsigned int getNumberOfAllNodes( void ) const { return m_numberOfNodes; }

        //! \brief return vertex number of a task graph vertex
        inline unsigned int getVertexNumber( void ) const { return m_vertexNumber; }

        //! \brief return vertex color of a task graph vertex
        inline unsigned int getVertexColor( void ) const { return m_vertexColor; }

        //! \brief return task graph vertex latency (process costs)
        inline const sc_time_t& getVertexLatency( void ) const { return m_vertexLatency; }


    protected:
        //! \var m_numberOfNodes
        //! \brief number of all collected nodes inside hierarchical task
        unsigned int m_numberOfNodes = {0};
        //! \var m_ProcessUnit
        //! \brief pointer to process unit which initialize and execute that vertex
        ProcessUnit_Base* const m_ProcessUnit;

    private:
        /************************************************************************/
        // Member
        /************************************************************************/
        //! \var m_vertexNumber
        //! \brief unique node identification number
        unsigned int m_vertexNumber;
        //! \var m_vertexColor
        //! \brief task graph node clustering color
        unsigned int m_vertexColor;
        //! \var m_vertexLatency
        //! \brief task graph node process costs
        sc_time_t m_vertexLatency;
        //! \var m_hierarchicalTask
        //! \brief shows that node is a hierarchical task vertex
        const bool m_hierarchicalTask = {true};
    };
}


#endif
