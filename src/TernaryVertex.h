//! \file TernaryVertex.h
//! \brief Ternary operator implementation as a task graph vertex

#ifndef TERNARYVERTEX_H_
#define TERNARYVERTEX_H_

#include "Typedefinitions.h"
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
    // TernaryVertex
    //!
    //! \class TernaryVertex
    //! \brief ternary operator task graph vertex
    //!
    //! \details 
    //! The ternary vertex returns the input values with id 0 and id 1 
    //! dependent on the boolean value of input with id 2.
    //! If the condition is true, the value with id 0 is returned, else the
    //! value with id 1.
    //! All inputs except of the input with id 2 are templates and the data type 
    //! can be chosen.
    //!
    //! \tparam T data type of return value by true condition
    //! \tparam U data type of return value by false condition
    //! \tparam O data type of outgoing edge
    /************************************************************************/

    template < typename T = int, typename U = T, typename O = T >
    class TernaryVertex : public sc_core::sc_module, public Task_Base
    {
    public:
        //! \brief constructor with sc_time object
        explicit TernaryVertex( ProcessUnit_Base* _pUnit, name_t _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, sc_time_t _latency )
            : sc_core::sc_module( _name ),
              Task_Base( std::string( _name ), _vertexNumber, _vertexColor, _latency ),
              m_coreFreeEv( ( this->getName( ) + "_coreFreeEv" ).c_str( ) ),
              m_ProcessUnit( _pUnit )
        {

            // set class type
            this->setClassType( typeid( *this ).name( ) );

            // register execution process at scheduler in elaboration phase
            sc_core::sc_spawn( sc_bind( &TernaryVertex::execute, this ),
                ( this->getName( ) + "_TernaryVertexProcess" ).c_str( ) );

            // generate input value synchronizations events
            m_inputEvVec.emplace_back(
                new event_t( ( this->getName( ) + "_inputOneEv" ).c_str( ) ) );
            m_inputEvVec.emplace_back(
                new event_t( ( this->getName( ) + "_inputTwoEv" ).c_str( ) ) );
            m_inputEvVec.emplace_back(
                new event_t( ( this->getName( ) + "_inputThreeEv" ).c_str( ) ) );
            // and list for process notification
            for ( auto& e : m_inputEvVec )
                m_exeProcEvAndList &= *e;

            // create Observer for input values
            inputObs.addObserver( m_inputEvVec[ 0 ].get( ),
                reinterpret_cast< dataPtr_t >( &m_inputOneVal.second ), sizeof( T ) );
            inputObs.addObserver( m_inputEvVec[ 1 ].get( ),
                reinterpret_cast< dataPtr_t >( &m_inputTwoVal.second ), sizeof( U ) );
            inputObs.addObserver( m_inputEvVec[ 2 ].get( ),
                reinterpret_cast< dataPtr_t >( &m_inputThreeVal.second ), sizeof( bool ) );
        }

        //! \brief constructor
        explicit TernaryVertex( ProcessUnit_Base* _pUnit, name_t _name, unsigned int _vertexNumber,
            unsigned int _vertexColor, double _latency, unit_t _unit )
            : TernaryVertex(
                  _pUnit, _name, _vertexNumber, _vertexColor, sc_time_t( _latency, _unit ) )
        {
        }

        //! \brief destructor
        virtual ~TernaryVertex( ) = default;

    private:
        // forbidden constructors:
        TernaryVertex( ) = delete; //!< \brief because vertexID should be unique
        TernaryVertex( const TernaryVertex& _source ) =
            delete; //!< \brief because sc_module could not be copied
        TernaryVertex( TernaryVertex&& _source ) =
            delete; //!< \brief because move not implemented for sc_module
        TernaryVertex& operator=( const TernaryVertex& _source ) = delete; //!< \brief forbidden
        TernaryVertex& operator=( TernaryVertex&& _source ) = delete;      //!< \brief forbidden

    public:
        /************************************************************************/
        // notifyObservers
        //!
        //! \brief notify all successor observers
        //!
        //! \param [in] _outputId id of return value
        //! 
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
                                        reinterpret_cast<dataPtr_t>( &m_returnOneVal.second ),
                                        sizeof( O ) );
                }
            }
        }

    public:
        /************************************************************************/
        // execute
        //!
        //! \brief do ternary operation
        /************************************************************************/

        virtual void execute( void ) override
        {
            while ( true )
                {
                    // parent value synchronization
                    sc_core::wait( m_exeProcEvAndList );

                    m_ProcessUnit->isCoreUsed( &m_coreFreeEv );
                    sc_core::wait( m_coreFreeEv );

                    m_returnOneVal.second = static_cast< O >(
                        m_inputThreeVal.second ? m_inputOneVal.second : m_inputTwoVal.second );


                    m_ProcessUnit->freeUsedCore( this->getVertexLatency( ) );

                    // notify children observers for output value
                    this->notifyObservers(m_returnOneVal.first);
                }
        }

    public:
        /************************************************************************/
        // getResults
        //!
        //! \brief get a tuple of all generated results
        //!
        //! \details 
        //! This function returns all generated outgoing values.
        //! The return is a tuple of all possible return values.
        //! The order is given by the id of the return value.
        //! A specific entry can be chosen by using the std::get-method with
        //! return value id as a template parameter.
        //!
        //! \return std::tuple of all generated outgoing values in the order of
        //! their ids
        /************************************************************************/

        std::tuple< O > getResults( void ) const
        {
            auto retVal = std::make_tuple( m_returnOneVal.second );
            return retVal;
        }

    public:
        //! \brief return kind of systemC module as string
        virtual const char* kind( ) const override { return "TernaryVertex"; }

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

        virtual void  end_of_simulation(void) override
        {
            std::cout << "Name " << this->getName() << std::endl;
            std::cout << "type: " << this->getClassType() << std::endl;
            std::cout << "last incoming values: " << m_inputOneVal.second << ", " << m_inputTwoVal.second << std::endl;
            std::cout << "condition: " << m_inputThreeVal.second << std::endl;
            std::cout << "vertex number: " << this->getVertexNumber() << std::endl;
            std::cout << "vertex color: " << this->getVertexColor() << std::endl;
            std::cout << "vertex latency: " << this->getVertexLatency().to_string() << std::endl;
            std::cout << "result: " << std::get<0>(getResults()) << std::endl;
            std::cout << "\n" << std::endl;
        }

    private:
        /************************************************************************/
        // Member
        /************************************************************************/

        //! \var numOfIns
        //! \brief number of incoming edges
        const unsigned int numOfIns = {3};
        //! \var numOfOuts
        //! \brief number of outgoing edges
        const unsigned int numOfOuts = {2};

        //! \var m_inputOneVal
        //! \brief return value for condition is true
        std::pair< unsigned int, T > m_inputOneVal = {0, 0};

        //! \var m_inputTwoVal
        //! \brief return value for condition is false
        std::pair< unsigned int, U > m_inputTwoVal = {1, 0};

        //! \var m_inputThreeVal
        //! \brief condition of ternary operator
        std::pair< unsigned int, bool > m_inputThreeVal = {2, false};

        //! \var m_returnOneVal
        //! \brief short description
        std::pair< unsigned int, O > m_returnOneVal = {0, 0};

        /************************************************************************/
        // scheduler synchronization events
        /************************************************************************/

        //! \var m_inputEvVec
        //! \brief vector of observer synchronization events
        std::vector< std::unique_ptr< event_t > > m_inputEvVec;
        //! \var m_coreFreeEv
        //! \brief core scheduler event for waiting queue
        event_t m_coreFreeEv;
        //! \var m_exeProcEvAndList
        //! \brief event and list for execute-method synchronization
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
