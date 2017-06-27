//! \file ProcessUnit_Base.h
//! \brief Standard functions and members of a process unit

#ifndef PROCESSUNIT_BASE_H_
#define PROCESSUNIT_BASE_H_

#include "Typedefinitions.h"
#include "Subject.h"
#include <queue>
#include <map>


namespace vc_utils
{
    /************************************************************************/
    //! \struct ProcessUnit_Base
    //!
    //! \brief Base class for process unit
    //!
    //! \details
    //! This struct contains all members and interfaces that a process unit
    //! should have. This includes methods and members for scheduling and
    //! the addition of vertices.
    //! A method for Observer connections between added tasks is also provided.
    //! A derived class has to implement memory and interconnect requirements.
    //!
    //!
    //! \attention
    //! If other nodes then Task_Base or IfVertex are constructed a specified
    //! add-method has to be implemented here.
    /************************************************************************/
    struct ProcessUnit_Base : public sc_core::sc_module
    {
        typedef std::map< unsigned int, Subject* > vertices_t;
        typedef std::queue< event_t* > eventQueue_t;

    public:
        virtual ~ProcessUnit_Base( ) = default;

    protected:
        /***************************************************************/
        // constructor:
        //!
        //! \brief    standard constructor
        //!
        //! \param [in] _name sc_module name
        //! \param [in] _unitId unique identification number of a process unit
        //!
        //!
        //! \author    Andre Werner
        //! \version   2015-7-15 : initial
        /***************************************************************/
        explicit ProcessUnit_Base( name_t _name, unsigned int _unitId );

    private:
        ProcessUnit_Base( ) = delete; //!< \brief forbidden constructor
        ProcessUnit_Base(
            const ProcessUnit_Base& _source ) = delete;          //!< \brief forbidden constructor
        ProcessUnit_Base( ProcessUnit_Base&& _source ) = delete; //!< \brief forbidden constructor
        ProcessUnit_Base& operator=(
            const ProcessUnit_Base& _rhs ) = delete; //!< \brief forbidden constructor
        ProcessUnit_Base& operator=(
            ProcessUnit_Base&& _rhs ) = delete; //!< \brief forbidden constructor

    public:
        /************************************************************************/
        /* process unit scheduler                                               */
        /************************************************************************/


        /***************************************************************/
        // isCoreUsed:
        //!
        //! \brief    checks if process unit is in use at the moment
        //!
        //! \param [in] _event coreFreeEv of a requesting task graph vertex
        //!
        //! \details
        //! The process unit should model a simple arithmetic unit. So
        //! It could only do one task at the same time.
        //! If more then one task is ready at the same time, the process
        //! unit have a waiting process queue to store all parallel
        //! execution requests and calculate them sequentially.
        //!
        //! \author    Andre Werner
        //! \version   2015-5-7 : initial
        /***************************************************************/
        void isCoreUsed( event_t* _event );

        /***************************************************************/
        // freeUsedCore:
        //!
        //! \brief    declare process unit as unused
        //!
        //! \param [in] _latency DESCRIPTION
        //!
        //! \details
        //! If the task execution is ready, the task call this function to
        //! set the core unit as free or to notify the next waiting task.
        //!
        //! \author    Andre Werner
        //! \version   2015-5-7 : initial
        /***************************************************************/
        void freeUsedCore( const sc_time_t& _latency );

        /************************************************************************/
        /* sc module functions                                                  */
        /************************************************************************/

        virtual const char* kind( ) const override;

        virtual void print(::std::ostream& os = ::std::cout ) const override;

        virtual void dump(::std::ostream& os = ::std::cout ) const override;

        /************************************************************************/
        /* system building functions                                            */
        /************************************************************************/

        /***************************************************************/
        // addVertex
        //!
        //! \func     addVertex
        //! \brief   add task graph vertex to process unit
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of the current process
        //! unit.
        //! The vertex is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it
        //! is
        //! used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT >
        unsigned int addVertex( unsigned int _id, const std::string _name, unsigned int _color,
            const sc_time_t& _latency )
        {
            auto tmp = m_vertices.emplace(
                _id, new vertexT( this, _name.c_str( ), _id, _color, _latency ) );

            return _id;
        }

        /***************************************************************/
        // addIfVertex
        //!
        //! \brief   add task graph IfVertex to process unit
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of the current process
        //! unit.
        //! The vertex is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it
        //! is
        //! used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT = IfVertex >
        unsigned int addIfVertex( unsigned int _vertexNumber, name_t _name,
            unsigned int _vertexColor, const sc_time_t& _latency, unsigned int _numOfInEdges,
            Subject* const _condition )
        {
            auto tmp = m_vertices.emplace(
                _vertexNumber, new vertexT( _name, this, _vertexColor, _vertexNumber, _latency,
                                   _numOfInEdges, _condition ) );

            return _vertexNumber;
        }


        /***************************************************************/
        // connect
        //!
        //! \brief    binds a observer on a subject
        //!
        //! \param [in] _dest Subject which is observed
        //! \param [in] _obs module which has an ObserverManager
        //! \param [in] _obsId Observer identification to choose a Observer from
        //! data field
        //! \param [in] _valId value identification to identify observed output of
        //! Subject
        //!
        //! \details
        //! This function binds a Observer from a module _obs on a Subject _dest.
        //!
        //! \tparam nodetypeT Set the type of module that includes an ObserverManager
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class nodetypeT >
        void connect( Subject* _sub, Subject* _obs, unsigned int _obsId, unsigned int _valId )
        {
            auto tmpObs = static_cast< nodetypeT* >( _obs )->inputObs.getObserver( _obsId );

            if ( tmpObs != nullptr )
                {
                    _sub->registerObserver( tmpObs, _valId );
                }
            else
                SC_REPORT_ERROR( this->name( ), "Observer not found." );
        }

    public:
        //! \var m_coreUsed
        //! \brief flag to set a process unit as used
        bool m_coreUsed = {false};
        //! \var m_unitId
        //! \brief unique process unit identification number
        unsigned int m_unitId;
        //! \var m_processWaitingQueue
        //! \brief waiting queue so serialize parallel access to process unit
        eventQueue_t m_processWaitingQueue;
        //! \var m_vertices
        //! \brief data field with all added vertices identified by there vertex id
        vertices_t m_vertices;
    };
}




#endif
