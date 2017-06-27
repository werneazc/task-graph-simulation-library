//! \file Observer.h
//! \brief implementation of Observer functionality


#ifndef OBSERVER_H_
#define OBSERVER_H_


#include <memory>
#include <systemc>
#include "Typedefinitions.h"



namespace vc_utils
{

    //! \class Observer
    //! \brief Object to observe value at parent Subject
    class Observer
    {
    public:
        /***************************************************************/
        // constructor:
        //!
        //! \brief	  	standard constructor
        //!
        //! \param [in]	_event	trigger event for task process, that
        // instantiates observer
        //! \param [in]	_value	pointer to memory that stores observed value
        //! \param [in]	_memSize	size of storage for incoming data
        //! \return    	:	Observer object
        //!
        //!	\details
        //!	Observer are instantiate in a task for every value the task
        // needs from his parents.
        //!	The _event is used to trigger a method process in a task.
        //!	The method process should have a AND-list as the trigger
        // parameters.
        //!	The _value points to a data storage in the task object and
        //_memSize describes
        //!	how big the storage is in bytes.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        explicit Observer( event_t* _event, dataPtr_t _value = nullptr, unsigned int _memSize = 0 )
            : m_event( _event ), m_valuePtr( _value ), m_memSize( _memSize )
        {
        }

        //! \brief destructor
        ~Observer( ) = default;

        /***************************************************************/
        // notify:
        //!
        //! \brief	  	value changed notification for observed value
        //!
        //! \param [in]	_latency	notification latency
        //! \param [in]	_data	pointer to variable at subject witch is observed
        //! \param [in]	_numOfBytes	number of bytes should be copied
        //!
        //!	\details
        //!	This method is called at a Subject if the observed value
        // changed.
        //!	The method copies the value in a local task variable where the
        // Observer is
        //!	instantiated and notify the value change by a local event
        // member.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        virtual void notify( const sc_time_t& _latency, dataPtr_t _data, std::size_t _numOfBytes )
        {
            sc_assert( ( m_valuePtr != nullptr ) && ( m_memSize >= _numOfBytes ) );

            memcpy( m_valuePtr, _data, _numOfBytes );

            m_event->notify( _latency );
        }

        //! \fn getValuePtr
        //! \brief get value copy destination
        dataPtr_t getValuePtr( ) const { return m_valuePtr; }

        //! \fn setValuePtr
        //! \brief set new value copy destination
        void setValuePtr( dataPtr_t _ptr ) { m_valuePtr = _ptr; }

        //! \fn notifySynchronisationEvent
        //! \brief notify synchronization event
        void notifySynchronisationEvent( const sc_time_t& _latency )
        {
            m_event->notify( _latency );
        }

        //! \fn getMemSize
        //! \brief return memory space for observed value copy destination
        unsigned int getMemSize( ) const { return m_memSize; }

    private:
        // forbidden constructors
        Observer( ) = delete;                                 //!< \brief forbidden
        Observer( const Observer& _source ) = delete;         //!< \brief forbidden
        Observer( Observer&& _source ) = delete;              //!< \brief forbidden
        Observer& operator=( const Observer& _rhs ) = delete; //!< \brief forbidden
        Observer& operator=( Observer&& _rhs ) = delete;      //!< \brief forbidden

    private:
        event_t* m_event;       //!< sc_event for trigger_next method AND-list in task process
        dataPtr_t m_valuePtr;   //!< pointer to task variable that stores observed value local
        unsigned int m_memSize; //!< size of local variable at task
    };
}

#endif