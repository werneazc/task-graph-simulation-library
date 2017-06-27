//! \file IfVertex.h
//! \brief Hierarchical task graph node to implement if-then-else

#ifndef IFVERTEX_H_
#define IFVERTEX_H_

#include "Hierarchical_Task.h"
#include "ObserverManager.h"
#include <vector>
#include <utility>
#include <set>
#include <map>

namespace vc_utils
{
    /************************************************************************/
    //! \class IfVertex
    //!
    //! \brief Hierarchical task graph node to implement if-then-else
    //!
    //! \details
    //! This class includes two sub classes for then and else path. 
    //! This is necessary, because children have to register their Observers
    //! at their parents but also if-successors have to do so. 
    //! To separate the nodes from both paths and also the if successors,
    //! the two path classes for then and else handle their included nodes.
    //! The if vertex representation use different functions to add and 
    //! connect other nodes inside both paths. 
    //! Included nodes, that use values from outside the if are registered 
    //! at their path classes for notification.
    //! If successors are registered at the if vertex.
    //! The if vertex doesn't store any values. He only knows the description 
    //! of values by a pointer to the begin of the value and the value size in bytes.
    /************************************************************************/
    class IfVertex : public sc_core::sc_module, public Hierarchical_Task
    {

    private:
        /************************************************************************/
        // type definitions:
        /************************************************************************/
        //! \typedef dataVec_t
        //! \brief stores begin of data and the data size in bytes
        typedef std::vector< std::pair< dataPtr_t, unsigned int > > dataVec_t;
        //! \typedef vertices_t
        //! \brief stores initialized vertices
        typedef std::map< unsigned int, Subject* > vertices_t;

    private:
        /************************************************************************/
        // then path
        /************************************************************************/
        /************************************************************************/
        //! \class ThenPath
        //!
        //! \brief then path of if-then-else representation
        //!
        //! \details
        //! Holds all vertices that are included in then path.
        //! Nodes that need values from outside the if scope have to register for them
        //! at then node. There are different functions provided at ifVertex class.
        /************************************************************************/
        class ThenPath : public Subject
        {

            // for easy access to vertices of then path
            friend class IfVertex;

            /************************************************************************/
            // constructor
            /************************************************************************/
        public:
            //! \brief constructor
            //! \param [in] _parentIf points to parent if vertex
            //! \param [in] _name name of ThenPath object
            explicit ThenPath( IfVertex* _parentIf, std::string _name );

            //! \brief destructor
            virtual ~ThenPath( ) = default;

        private:
            // forbidden constructors
            ThenPath( ) = delete;                                 //!< \brief forbidden constructor
            ThenPath( const ThenPath& _source ) = delete;         //!< \brief forbidden constructor
            ThenPath( ThenPath&& _source ) = delete;              //!< \brief forbidden constructor
            ThenPath& operator=( const ThenPath& _rhs ) = delete; //!< \brief forbidden constructor
            ThenPath& operator=( ThenPath&& _ths ) = delete;      //!< \brief forbidden constructor

        public:
            /************************************************************************/
            // methods for structure building
            /************************************************************************/
            /***************************************************************/
            // addVertex
            //!
            //! \brief   add task graph vertex to then path
            //!
            //! \param [in] _id task graph vertex identification number
            //!                 and vertex key in map where it is emplaced
            //! \param [in] _pUnit owner of the task graph vertex
            //! \param [in] _name sc_module name
            //! \param [in] _color clustering color of vertex
            //! \param [in] _latency process latency of the task graph vertex
            //!
            //! \details
            //! This function adds a vertex into the vertex map of the current then path.
            //! The vertex type is described by the template parameter vertexT.
            //! The data field is a map, so the vertex number has to be unique because it is
            //! used as the key of the map. If a vertex with _id already exists, the
            //! program ends with an error.
            //!
            //! \tparam  vertexT type of generated vertex
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class vertexT >
            void addVertex( unsigned int _id, ProcessUnit_Base* _pUnit, const std::string _name,
                unsigned int _color, const sc_time_t _latency )
            {
                auto tmp = m_vertices.emplace(
                    _id, new vertexT( _pUnit, _name.c_str( ), _id, _color, _latency ) );

                if ( !tmp.second )
                    SC_REPORT_ERROR( this->getName_Cstr( ),
                        "The vertex with given id already exits. Vertex is not emplaced." );
            }

        public:
            /***************************************************************/
            // addIfVertex
            //!
            //! \brief   add if-vertex to then path
            //!
            //! \param [in] _id task graph vertex identification number
            //! \param [in] _unit owner of the task graph vertex
            //! \param [in] _name sc_module name
            //! \param [in] _color clustering color of vertex
            //! \param [in] _latency process latency of the task graph vertex
            //!
            //! \details
            //! This function adds a vertex into the vertex map of the current
            //! if-then-path.
            //! The vertex is described by the template parameter vertexT.
            //! The data field is a map, so the vertex number has to be unique because it
            //! is used as the key of the map.
            //!
            //! \tparam  vertexT type of generated vertex.
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class vertexT = IfVertex >
            void addIfVertex( unsigned int _vertexNumber, name_t _name, ProcessUnit_Base* _unit,
                unsigned int _vertexColor, sc_time_t _latency, unsigned int _numOfInEdges,
                Subject* const _condition )
            {
                auto tmp = m_vertices.emplace(
                    _vertexNumber, new vertexT( _name, _unit, _vertexColor, _vertexNumber, _latency,
                                       _numOfInEdges, _condition ) );
            }

        public:
            /***************************************************************/
            // connect
            //!
            //! \brief    binds a observer on a subject
            //!
            //! \param [in] _sub Subject which is observed
            //! \param [in] _obs module which has an ObserverManager
            //! \param [in] _obsId Observer identification number
            //!              to choose a Observer from data field in ObserverManager
            //! \param [in] _valId value identification to identify observed output of Subject
            //!
            //! \details
            //! This function binds a Observer from a module _obs on a Subject _sub.
            //!
            //! \tparam nodeTypeT Set the type of module that includes an ObserverManager
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class nodeTypeT >
            void connect( Subject* _sub, Subject* _obs, unsigned int _obsId, unsigned int _valId )
            {
                auto tmpObs = static_cast< nodeTypeT* > _obs->inputObs.getObserver( _obsId );

                if ( tmpObs != nullptr )
                    {
                        _sub->registerObserver( tmpObs, _valId );
                    }
                else
                    SC_REPORT_ERROR( this->getName_Cstr( ), "Observer not found." );
            }


        public:
            /************************************************************************/
            // successor notification
            /************************************************************************/
            /***************************************************************/
            // notifyObservers
            //!
            //! \brief    notify nodes in then path
            //!
            //! \param [in] _outValueId Id of value, successors should be notified for
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            virtual void notifyObservers( unsigned int _outValueId ) override;

        private:
            /************************************************************************/
            // all initialized vertices
            /************************************************************************/
            vertices_t m_vertices;
            //!< \brief stores vertices for all secondary process separated by vertex id

            /************************************************************************/
            // member
            /************************************************************************/
            //! \var m_parentIf
            //! \brief pointer to parent if node that includes that then path
            IfVertex* const m_parentIf;
        };

    private:
        /************************************************************************/
        // else path
        /************************************************************************/
        /************************************************************************/
        //! \class ElsePath
        //!
        //! \brief else path of if-then-else representation
        //!
        //! \details
        //! Holds all vertices that are included in else path.
        //! Nodes that need values from outside the if scope have to register for them
        //! at else node. There are different functions provided at ifVertex class.
        /************************************************************************/
        class ElsePath : public Subject
        {

            // for easy access to vertices of then path
            friend class IfVertex;

            /************************************************************************/
            // constructor
            /************************************************************************/
        public:
            //! \brief constructor
            //! \param [in] _parentIf points to parent if vertex
            //! \param [in] _name name of ElsePath object
            explicit ElsePath( IfVertex* _parentIf, std::string _name );

            //! \brief destructor
            virtual ~ElsePath( ) = default;

        private:
            // forbidden constructors
            ElsePath( ) = delete;                                 //!< \brief forbidden constructor
            ElsePath( const ElsePath& _source ) = delete;         //!< \brief forbidden constructor
            ElsePath( ElsePath&& _source ) = delete;              //!< \brief forbidden constructor
            ElsePath& operator=( const ElsePath& _rhs ) = delete; //!< \brief forbidden constructor
            ElsePath& operator=( ElsePath&& _ths ) = delete;      //!< \brief forbidden constructor

        public:
            /************************************************************************/
            // methods for structure building
            /************************************************************************/
            /***************************************************************/
            // addVertex
            //!
            //! \brief   add task graph vertex to else path
            //!
            //! \param [in] _id task graph vertex identification number
            //!                 and vertex key in map where it is emplaced
            //! \param [in] _pUnit owner of the task graph vertex
            //! \param [in] _name sc_module name
            //! \param [in] _color clustering color of vertex
            //! \param [in] _latency process latency of the task graph vertex
            //!
            //! \details
            //! This function adds a vertex into the vertex map of the current else path.
            //! The vertex type is described by the template parameter vertexT.
            //! The data field is a map, so the vertex number has to be unique because it is
            //! used as the key of the map. If a vertex with _id already exists, the
            //! program ends with an error.
            //!
            //! \tparam  vertexT type of generated vertex
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class vertexT >
            void addVertex( unsigned int _id, ProcessUnit_Base* _pUnit, const std::string _name,
                unsigned int _color, const sc_time_t _latency )
            {
                auto tmp = m_vertices.emplace(
                    _id, new vertexT( _pUnit, _name.c_str( ), _id, _color, _latency ) );

                if ( !tmp.second )
                    SC_REPORT_ERROR( this->getName_Cstr( ),
                        "The vertex with given id already exits. Vertex is not emplaced." );
            }

        public:
            /***************************************************************/
            // addIfVertex
            //!
            //! \brief   add if-vertex to else path
            //!
            //! \param [in] _id task graph vertex identification number
            //! \param [in] _unit owner of the task graph vertex
            //! \param [in] _name sc_module name
            //! \param [in] _color clustering color of vertex
            //! \param [in] _latency process latency of the task graph vertex
            //!
            //! \details
            //! This function adds a vertex into the vertex map of the current
            //! if-then-path.
            //! The vertex is described by the template parameter vertexT.
            //! The data field is a map, so the vertex number has to be unique because it
            //! is used as the key of the map.
            //!
            //! \tparam  vertexT type of generated vertex.
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class vertexT = IfVertex >
            void addIfVertex( unsigned int _vertexNumber, name_t _name, ProcessUnit_Base* _unit,
                unsigned int _vertexColor, sc_time_t _latency, unsigned int _numOfInEdges,
                Subject* const _condition )
            {
                auto tmp = m_vertices.emplace(
                    _vertexNumber, new vertexT( _name, _unit, _vertexColor, _vertexNumber, _latency,
                                       _numOfInEdges, _condition ) );
            }


        public:
            /***************************************************************/
            // connect
            //!
            //! \brief    binds a observer on a subject
            //!
            //! \param [in] _sub Subject which is observed
            //! \param [in] _obs module which has an ObserverManager
            //! \param [in] _obsId Observer identification number
            //!              to choose a Observer from data field in ObserverManager
            //! \param [in] _valId value identification to identify observed output of Subject
            //!
            //! \details
            //! This function binds a Observer from a module _obs on a Subject _dest.
            //!
            //! \tparam nodeTypeT Set the type of module that includes an ObserverManager
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            template < class nodeTypeT >
            void connect( Subject* _sub, Subject* _obs, unsigned int _obsId, unsigned int _valId )
            {
                auto tmpObs = static_cast< nodeTypeT* > _obs->inputObs.getObserver( _obsId );

                if ( tmpObs != nullptr )
                    {
                        _sub->registerObserver( tmpObs, _valId );
                    }
                else
                    SC_REPORT_ERROR( this->getName_Cstr( ), "Observer not found." );
            }


        public:
            /************************************************************************/
            // successor notification
            /************************************************************************/
            /***************************************************************/
            // notifyObservers
            //!
            //! \brief    binds a observer on a subject
            //!
            //! \param [in] _sub Subject which is observed
            //! \param [in] _obs module which has an ObserverManager
            //! \param [in] _obsId Observer identification to choose a Observer from data field
            //! \param [in] _valId value identification to identify observed output of Subject
            //!
            //! \details
            //! This function binds a Observer from a module _obs on a Subject _dest.
            //!
            //! \tparam typeT Set the type of module that includes an ObserverManager
            //!
            //! \author    Andre Werner
            //! \version   2015-6-14 : initial
            /***************************************************************/
            virtual void notifyObservers( unsigned int _outValueId ) override;

        private:
            /************************************************************************/
            // all initialized vertices
            /************************************************************************/
            vertices_t m_vertices;
            //!< \brief stores vertices for all secondary process separated by pixel and vertex id

            /************************************************************************/
            // member
            /************************************************************************/
            //! \var m_parentIf
            //! \brief pointer to parent if node that includes that then path
            IfVertex* const m_parentIf;
        };

        /************************************************************************/
        // friends
        /************************************************************************/
    private:
        friend class ThenPath;
        friend class ElsePath;

    public:
        /************************************************************************/
        // ObserverManager
        /************************************************************************/
        //! \var inputObs
        //! \brief ObserverManager for all if node incoming edges except of condition
        ObserverManager< ObserverInterconnect > inputObs;

    private:
        //! \var m_conditionObs
        //! \brief ObserverManager for condition only
        ObserverManager< Observer > m_conditionObs;
        //! \var m_ifEndObs
        //! \brief ObserverManager for node results that are dependencies for if successor
        ObserverManager< ObserverInterconnect > m_ifEndObs;


    public:
        /************************************************************************/
        // constructor
        /************************************************************************/
        SC_HAS_PROCESS( IfVertex );

        /***************************************************************/
        // IfVertex
        //!
        //! \brief   add task graph vertex to hierarchical task
        //!
        //! \param [in] _vertexNumber task graph vertex identification number
        //!                 and vertex key in map where it is emplaced
        //! \param [in] _unit owner of the if vertex
        //! \param [in] _name sc_module name
        //! \param [in] _numOfInEdges number of dependencies except of condition
        //! \param [in] _vertexColor clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        explicit IfVertex( name_t _name, ProcessUnit_Base* _unit, unsigned int _vertexColor,
            unsigned int _vertexNumber, sc_time_t _latency, unsigned int _numOfInEdges,
            Subject* const _condition );

        //! \brief destructor
        virtual ~IfVertex( );

    private:
        // forbidden constructors
        IfVertex( ) = delete;                                 //!< \brief forbidden constructor
        IfVertex( const IfVertex& _source ) = delete;         //!< \brief forbidden constructor
        IfVertex( IfVertex&& _source ) = delete;              //!< \brief forbidden constructor
        IfVertex& operator=( const IfVertex& _rhs ) = delete; //!< \brief forbidden constructor
        IfVertex& operator=( IfVertex&& _rhs ) = delete;      //!< \brief forbidden constructor

    public:
        /************************************************************************/
        // SystemC process
        /************************************************************************/
        /***************************************************************/
        // conditionCheck
        //!
        //! \brief    condition check to run then or else path
        //!
        //! \details
        //! All nodes, that need some of the incoming dependencies are
        //! notified in the path that is chosen.
        //!
        //! \author    Andre Werner
        //! \version   2015-7-5 : initial
        /***************************************************************/
        void conditionCheck( void );


        /***************************************************************/
        // ifEndFromThenProcess
        //!
        //! \brief    end of if vertex
        //!
        //! \details
        //! This process waits for all dependencies of the if node successors.
        //! After all values are ready, the observers of the successors are
        //! notified.
        //!
        //! \author    Andre Werner
        //! \version   2015-7-5 : initial
        /***************************************************************/
        void ifEndFromThenProcess( void );

        /***************************************************************/
        // ifEndFromElseProcess
        //!
        //! \brief    end of if vertex
        //!
        //! \details
        //! This process waits for all dependencies of the if node successors.
        //! After all values are ready, the observers of the successors are
        //! notified.
        //!
        //! \author    Andre Werner
        //! \version   2015-7-5 : initial
        /***************************************************************/
        void ifEndFromElseProcess( void );


    public:
        /************************************************************************/
        // methods for structure building then path
        /************************************************************************/
        /***************************************************************/
        // addVertexToThen
        //!
        //! \brief   add task graph node to then path
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of then path.
        //! The vertex type is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it is
        //! used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT >
        void addVertexToThen( unsigned int _id, const std::string _name, unsigned int _color,
            const sc_time_t _latency )
        {
            m_thenPath.addVertex< vertexT >( _id, m_ProcessUnit, _name, _color, _latency );

            // count number of vertices
            m_numberOfNodes++;
        }

        /***************************************************************/
        // addIfVertexToThen
        //!
        //! \brief   add if-vertex to then path
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _unit owner of the task graph vertex
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of the current
        //! if-then-path.
        //! The vertex is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it
        //! is used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT = IfVertex >
        void addIfVertexToThen( unsigned int _vertexNumber, name_t _name,
            unsigned int _vertexColor, sc_time_t _latency, unsigned int _numOfInEdges,
            Subject* const _condition )
        {
            m_thenPath.addIfVertex(
                _vertexNumber, _name, m_ProcessUnit, _vertexColor, _latency, _numOfInEdges, _condition );

            // count number of vertices
            m_numberOfNodes++;
        }

        /***************************************************************/
        // connectInsideThenPath
        //!
        //! \brief    binds a observer on a subject
        //!
        //! \param [in] _subNodeId node identification number for subject
        //! \param [in] _obsNodeId node identification number for observer
        //! \param [in] _obsId Observer identification number to choose a Observer
        //!             from ObserverManager
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! This function binds a Observer from a module on a Subject.
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class nodeTypeT >
        void connectInsideThenPath( unsigned int _subNodeId, unsigned int _obsNodeId,
            unsigned int _obsId, unsigned int _valId )
        {

            // check if nodes exist inside of then path
            if ( !( m_thenPath.m_vertices.count( _subNodeId ) &&
                     m_thenPath.m_vertices.count( _obsNodeId ) ) )
                SC_REPORT_ERROR(
                    this->name( ), "no valid identification number for node in then path" );

            auto subNode = m_thenPath.m_vertices[ _subNodeId ];
            auto obsNode = m_thenPath.m_vertices[ _obsNodeId ];
            // connect nodes inside of then path
            m_thenPath.connect< nodeTypeT >( subNode, obsNode, _obsId, _valId );
        }

        /***************************************************************/
        // connectToIfInDependency
        //!
        //! \brief    binds a observer to if vertex incoming dependency
        //!
        //! \param [in] _obsNodeId node identification number for observer
        //! \param [in] _obsId Observer identification number to choose a Observer
        //!             from ObserverManager
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! This function binds a Observer from a module to then path for notification.
        //! Use this method to register the first nodes of a core code sequence placed in
        //! if-then-path. The _valId is equal to the observer ID which observes this value
        //! at ifBegin-Vertex.    
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class nodeTypeT >
        void connectToThenDependency(
            unsigned int _obsNodeId, unsigned int _obsId, unsigned int _valId )
        {
            if ( !m_thenPath.m_vertices.count( _obsNodeId ) )
                SC_REPORT_ERROR(
                    this->name( ), "no valid identification number for node in then path" );

            auto obsPtr = static_cast< nodeTypeT* >( m_thenPath.m_vertices[ _obsNodeId ] )
                              ->inputObs.getObserver( _obsId );
            m_thenPath.registerObserver( obsPtr, _valId );

            // register valId for notifications in the condition check process
            m_thenNodes.emplace( _valId );
        }

        /***************************************************************/
        // registerThenOutDependency
        //!
        //! \brief    binds a observer on a subject
        //!
        //! \param [in] _subNodeId node identification number for subject
        //! \param [in] _inEdgeId identification number of similar value at
        //!             incoming if-vertex dependencies
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! Use that method to register if vertex successor dependencies.
        //! When the if-then-path core code sequence if finished, registered successors
        //! will be notified. _subNodeId is the vertex number of a node that is
        //! implemented in the then-path and modifies a successor dependency of the
        //! if-node for the last time in the core code sequence.
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        void registerThenOutDependency(
            unsigned int _subNodeId, unsigned int _inEdgeId, unsigned int _valId );

    public:
        /************************************************************************/
        // methods for structure building else path
        /************************************************************************/
        /***************************************************************/
        // addVertexToElse
        //!
        //! \brief   add task graph node to else path
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of else path.
        //! The vertex type is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it is
        //! used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT >
        void addVertexToElse( unsigned int _id, const std::string _name, unsigned int _color,
            const sc_time_t _latency )
        {
            m_elsePath.addVertex< vertexT >( _id, m_ProcessUnit, _name, _color, _latency );

            // count number of vertices
            m_numberOfNodes++;
        }

    public:
        /***************************************************************/
        // addIfVertexToElse
        //!
        //! \brief   add if-vertex to else path
        //!
        //! \param [in] _id task graph vertex identification number
        //! \param [in] _unit owner of the task graph vertex
        //! \param [in] _name sc_module name
        //! \param [in] _color clustering color of vertex
        //! \param [in] _latency process latency of the task graph vertex
        //!
        //! \details
        //! This function adds a vertex into the vertex map of the current
        //! if-then-path.
        //! The vertex is described by the template parameter vertexT.
        //! The data field is a map, so the vertex number has to be unique because it
        //! is used as the key of the map.
        //!
        //! \tparam  vertexT type of generated vertex.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class vertexT = IfVertex >
        void addIfVertexToElse( unsigned int _vertexNumber, name_t _name,
            unsigned int _vertexColor, sc_time_t _latency, unsigned int _numOfInEdges,
            Subject* const _condition )
        {
            m_elsePath.addIfVertex(
                _vertexNumber, _name, m_ProcessUnit, _vertexColor _latency, _numOfInEdges, _condition );

            // count number of vertices
            m_numberOfNodes++;
        }


        /***************************************************************/
        // connectInsideElsePath
        //!
        //! \brief    binds a observer on a subject
        //!
        //! \param [in] _subNodeId node identification number for subject
        //! \param [in] _obsNodeId node identification number for observer
        //! \param [in] _obsId Observer identification number to choose a Observer
        //!             from ObserverManager
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! This function binds a Observer from a module on a Subject.
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class nodeTypeT >
        void connectInsideElsePath( unsigned int _subNodeId, unsigned int _obsNodeId,
            unsigned int _obsId, unsigned int _valId )
        {

            // check if nodes exist inside of then path
            if ( !( m_elsePath.m_vertices.count( _subNodeId ) &&
                     m_elsePath.m_vertices.count( _obsNodeId ) ) )
                SC_REPORT_ERROR(
                    this->name( ), "no valid identification number for node in then path" );

            auto subNode = m_elsePath.m_vertices[ _subNodeId ];
            auto obsNode = m_elsePath.m_vertices[ _obsNodeId ];
            // connect nodes inside of then path
            m_elsePath.connect< nodeTypeT >( subNode, obsNode, _obsId, _valId );
        }

        /***************************************************************/
        // connectToElseDependency
        //!
        //! \brief    binds a observer to if vertex incoming dependency
        //!
        //! \param [in] _obsNodeId node identification number for observer
        //! \param [in] _obsId Observer identification number to choose a Observer
        //!             from ObserverManager
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! This function binds a Observer from a module to else path for notification.
        //! Use this method to register the first nodes of a core code sequence placed in
        //! if-else-path.
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        template < class nodeTypeT >
        void connectToElseDependency(
            unsigned int _obsNodeId, unsigned int _obsId, unsigned int _valId )
        {
            if ( !m_elsePath.m_vertices.count( _obsNodeId ) )
                SC_REPORT_ERROR(
                    this->name( ), "no valid identification number for node in then path" );

            auto obsPtr = static_cast< nodeTypeT* >( m_elsePath.m_vertices[ _obsNodeId ] )
                              ->inputObs.getObserver( _obsId );
            m_elsePath.registerObserver( obsPtr, _valId );

            // register valId for notifications in the condition check process
            m_elseNodes.emplace( _valId );
        }

        /***************************************************************/
        // registerElseOutDependency
        //!
        //! \brief    binds a observer on a subject
        //!
        //! \param [in] _subNodeId node identification number for subject
        //! \param [in] _inEdgeId identification number of similar value at
        //!             incoming if-vertex dependencies
        //! \param [in] _valId value identification number to identify observed
        //!             output of Subject
        //!
        //! \details
        //! Use that method to register if vertex successor dependencies.
        //! When the if-else-path core code sequence if finished, registered successors
        //! will be notified. _subNodeId is the vertex number of a node that is
        //! implemented in the else-path and modifies a  successor dependency of the
        //! if-node for the last time in the core code sequence.
        //!
        //! \tparam nodeTypeT Set the type of module that includes an ObserverManager.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        void registerElseOutDependency(
            unsigned int _subNodeId, unsigned int _inEdgeId, unsigned int _valId );


    public:
        /***************************************************************/
        // notifyObservers
        //!
        //! \brief   notify Observer of if-vertex successor
        //!
        //! \param [in] _outValueId value identification number for which
        //!             a Observer is registered
        //!
        //! \details
        //!
        //!
        //! \author    Andre Werner
        //! \version   2015-7-5 : initial
        /***************************************************************/
        virtual void notifyObservers( unsigned int _outValueId ) override;

    public:
        /************************************************************************/
        // SystemC sc_object methods
        /************************************************************************/
        //! \brief return kind of systemC module as string
        inline virtual const char* kind( ) const override { return "ifVertex"; }

        //! \brief return name of systemC module as string
        inline virtual void print(::std::ostream& os = ::std::cout ) const override
        {
            os << this->name( );
        }

        //! \brief return name and kind of systemC module as string
        inline virtual void dump(::std::ostream& os = ::std::cout ) const override
        {
            os << this->name( ) << ", " << this->kind( );
        }

    public:
        /************************************************************************/
        // getter
        /************************************************************************/
        //! \brief get pointer so then path node
        Subject* const getThenPathNode( unsigned int _vertexId );
        //! \brief get pointer so else path node
        Subject* const getElsePathNode( unsigned int _vertexId );

    private:
        /************************************************************************/
        // Member
        /************************************************************************/
        //! \var m_condition
        //! \brief if condition to take then (true) or else (false) path
        bool m_condition = { true };
        //! \var m_elseNodes
        //! \brief identification numbers of all edges for then path
        std::set< unsigned int > m_thenNodes;
        //! \var m_elseNodes
        //! \brief identification numbers of all edges for else path
        std::set< unsigned int > m_elseNodes;

    private:
        /************************************************************************/
        // path
        /************************************************************************/
        //! \var m_thenPath
        //! \brief includes all vertices that are used in then path
        ThenPath m_thenPath;
        //! \var m_elsePath
        //! \brief includes all vertices that are used in else path
        ElsePath m_elsePath;

    private:
        /************************************************************************/
        // data of observed predecessors
        /************************************************************************/
        //! \var m_ifBeginDataVec
        //! \brief saves pairs of data pointer and data length for all incoming edges
        dataVec_t m_ifBeginDataVec;
        //! \var m_ifEndDataVec
        //! \brief saves pairs of data pointer and data length for all outgoing edges
        dataVec_t m_ifEndDataVec;
    
    private:
        /************************************************************************/
        // SystemC synchronization events
        /************************************************************************/
        //! \var m_ifBeginEvVec
        //! \brief event data field for all incoming edges
        std::vector< event_t* > m_ifBeginEvVec;
        //! \var m_ifEndEvVec
        //! \brief event data field for all outgoing edges
        std::vector< event_t* > m_ifEndEvVec;
        //! \var m_ifBeginEvAndList
        //! \brief event-and-list to synchronize conditionCheck process
        sc_core::sc_event_and_list m_ifBeginEvAndList;
        //! \var m_ifEndFromThenEvAndList
        //! \brief event-and-list to synchronize end if process by using then path
        sc_core::sc_event_and_list m_ifEndFromThenEvAndList;
        //! \var m_ifEndFromElseEvAndList
        //! \brief event-and-list to synchronize end if process by using else path
        sc_core::sc_event_and_list m_ifEndFromElseEvAndList;
        //! \var m_conditionEv
        //! \brief event to notify availability of condition value
        event_t m_conditionEv;

    };
}
#endif // IFVERTEX_H_