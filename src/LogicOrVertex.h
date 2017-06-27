//! \file LogicOrVertex.h
//! \brief short description

#ifndef LOGICORVERTEX_H_
#define LOGICORVERTEX_H_

#include "Task_Base.h"
#include <utility>
#include <memory>
#include <tuple>

namespace vc_utils
{

    /************************************************************************/
    // declarations:
    class Observer;
    struct ProcessUnit_Base;
    /************************************************************************/

    /************************************************************************/
    // LogicOrVertex
    //!
    //! \class LogicOrVertex
    //! \brief short description
    //!
    //! \details detailed description
    //!
    //! \tparam template parameter description
    //! \tparam template parameter description
    /************************************************************************/

    template < typename T = int, typename G = T >
    class LogicOrVertex : public sc_core::sc_module, public Task_Base
    {
    public:
        //! \brief constructor with sc_time object
        explicit LogicOrVertex( ProcessUnit_Base* _pUnit, name_t _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, const sc_time_t& _latency )
            : sc_core::sc_module( _name ),
              Task_Base( std::string( _name ), _vertexNumber, _vertexColor, _latency ),
              m_coreFreeEv( ( this->getName( ) + "_coreFreeEv" ).c_str( ) ),
              m_ProcessUnit( _pUnit )
        {

            // set class type
            this->setClassType( typeid( *this ).name( ) );

            // register execution process at scheduler in elaboration phase
            sc_core::sc_spawn( sc_bind( &LogicOrVertex::execute, this ),
                ( this->getName( ) + "_LogicOrVertexProcess" ).c_str( ) );

            // generate input value synchronizations events
            m_inputEvVec.emplace_back(
                new event_t( ( this->getName( ) + "_inputOneEv" ).c_str( ) ) );
            m_inputEvVec.emplace_back(
                new event_t( ( this->getName( ) + "_inputTwoEv" ).c_str( ) ) );
            // and list for process notification
            for ( auto& e : m_inputEvVec )
                m_exeProcEvAndList &= *e;

            // create Observer for input values
            inputObs.addObserver( m_inputEvVec[ vc_utils::SIDE::LHS ].get( ),
                reinterpret_cast< dataPtr_t >( &m_inputOneVal.second ), sizeof( T ) );
            inputObs.addObserver( m_inputEvVec[ vc_utils::SIDE::RHS ].get( ),
                reinterpret_cast< dataPtr_t >( &m_inputTwoVal.second ), sizeof( G ) );
        }

        //! \brief constructor
        explicit LogicOrVertex( ProcessUnit_Base* _pUnit, name_t _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, double _latency, unit_t _unit )
            : LogicOrVertex(
                  _pUnit, _name, _vertexNumber, _vertexColor, sc_time_t( _latency, _unit ) )
        {
        }

        //! \brief destructor
        virtual ~LogicOrVertex( ) = default;

    private:
        // forbidden constructors:
        LogicOrVertex( ) = delete; //!< \brief because vertexID should be unique
        LogicOrVertex( const LogicOrVertex& _source ) =
            delete; //!< \brief because sc_module could not be copied
        LogicOrVertex( LogicOrVertex&& _source ) =
            delete; //!< \brief because move not implemented for sc_module
        LogicOrVertex& operator=( const LogicOrVertex& _source ) = delete; //!< \brief forbidden
        LogicOrVertex& operator=( LogicOrVertex&& _source ) = delete;      //!< \brief forbidden

    public:
        /************************************************************************/
        // notifyObservers
        //!
        //! \brief short description
        //!
        //! \details detailed description
        //!
        //! \param [in] parameter description
        //! \return return value description
        /************************************************************************/

        virtual void notifyObservers( unsigned int _outputId ) override
        {
            // check that ID for output is available
            sc_assert( numOfOuts > _outputId );
            // search for every Observer that is sensitive for value changes at _outputID
            for ( auto _obs : this->m_observerVec )
                {
                    if ( _obs.second == _outputId )
                        {
                            _obs.first->notify( sc_core::SC_ZERO_TIME,
                                reinterpret_cast< dataPtr_t >( &m_returnOneVal.second ),
                                sizeof( bool ) );
                        }
                }
        }

    public:
        /************************************************************************/
        // execute
        //!
        //! \brief short description
        //!
        //! \details detailed description
        //!
        //! \return return value description
        /************************************************************************/

        virtual void execute( void ) override
        {
            while ( true )
                {
                    // parent value synchronization
                    sc_core::wait( m_exeProcEvAndList );

                    m_ProcessUnit->isCoreUsed( &m_coreFreeEv );
                    sc_core::wait( m_coreFreeEv );

                    m_returnOneVal.second =
                        static_cast< bool >( m_inputOneVal.second || m_inputTwoVal.second );

                    m_ProcessUnit->freeUsedCore( this->getVertexLatency( ) );

                    // notify children observers for output value
                    notifyObservers( m_returnOneVal.first );
                }
        }

    public:
        /************************************************************************/
        // getResults
        //!
        //! \brief short description
        //!
        //! \details detailed description
        //!
        //! \return return value description
        /************************************************************************/

        std::tuple< bool > getResults( void ) const
        {
            auto retVal = std::make_tuple( m_returnOneVal.second );
            return retVal;
        }

    public:
        //! \brief return kind of systemC module as string
        virtual const char* kind( ) const override { return "LogicOrVertex"; }

        //! \brief return kind of systemC module as string
        virtual void print(::std::ostream& os = ::std::cout ) const override
        {
            os << this->name( );
        }

        //! \brief return name and kind of systemC module as string
        virtual void dump(::std::ostream& os = ::std::cout ) const override
        {
            os << this->name( ) << ", " << this->getClassType( );
        }

    private:
        /************************************************************************/
        // Member
        /************************************************************************/

        //! \var numOfIns
        //! \brief short description
        const unsigned int numOfIns = {2};
        //! \var numOfOuts
        //! \brief short description
        const unsigned int numOfOuts = {1};

        //! \var m_inputOneVal
        //! \brief short description
        std::pair< unsigned int, T > m_inputOneVal = {vc_utils::SIDE::LHS, 0};

        //! \var m_inputTwoVal
        //! \brief short description
        std::pair< unsigned int, G > m_inputTwoVal = {vc_utils::SIDE::RHS, 0};

        //! \var m_returnOneVal
        //! \brief short description
        std::pair< unsigned int, bool > m_returnOneVal = {0, 0};

        /************************************************************************/
        // scheduler synchronization events
        /************************************************************************/

        //! \var m_inputEvVec
        //! \brief short description
        std::vector< std::unique_ptr< event_t > > m_inputEvVec;
        //! \var m_coreFreeEv
        //! \brief short description
        event_t m_coreFreeEv;
        //! \var m_exeProcEvAndList
        //! \brief short description
        sc_core::sc_event_and_list m_exeProcEvAndList;

        /************************************************************************/
        // interface to process unit
        /************************************************************************/

        //! \var m_ProcessUnit
        //! \brief short description
        ProcessUnit_Base* const m_ProcessUnit;
    };
} // end of namespace vc_utils

#endif
