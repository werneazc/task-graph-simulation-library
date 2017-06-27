//! \file ObserverInterconnect.h
//! \brief specialized Observer for connections between task module and Interconnect

#ifndef OBSERVERINTERCONNECT_H_
#define OBSERVERINTERCONNECT_H_

#include "Observer.h"

namespace vc_utils
{
    /************************************************************************/
    // ObserverInterconnect
    //!
    //! \class ObserverInterconnect
    //!
    //! \brief specialized Observer for connections between task module and Interconnect
    /************************************************************************/
    class ObserverInterconnect : public Observer
    {
    public:
        //! \fn ObserverInterconnect
        //! \brief constructor
        explicit ObserverInterconnect(
            event_t* _event, dataPtr_t _value = nullptr, unsigned int _memSize = 0 )
            : Observer( _event, _value, _memSize ), m_valueChanged( false )
        {
        }

        //! \fn ObserverInterconnect
        //! \brief destructor
        ~ObserverInterconnect( ) = default;


        /***************************************************************/
        // notify
        //!
        //! \fn       notify
        //! \brief    notify Observer at Interconnect
        //!
        //! \param [in] _latency latency for module synchronization event
        //! \param [in] _data byte pointer to copied data
        //! \param [in] _numOfBytes number of bytes to be copied
        //!
        //! \details
        //! This method doesn't copy any value. It stores data pointer and number of
        //! bytes of the observed value for data transport between process units.
        //! The necessary values are stored in a standard pair.
        //!
        //! \author    Andre Werner
        //! \version   2015-6-6 : initial
        /***************************************************************/
        void notify( const sc_time_t& _latency, dataPtr_t _data, std::size_t _numOfBytes ) override
        {
            sc_assert( ( getValuePtr( ) != nullptr ) && ( getMemSize( ) >= _numOfBytes ) );

            // build information tuple for data source and number of bytes has to be copied
            auto tmp = std::make_pair( _data, _numOfBytes );

            // save values in interconnect to generate external communication
            memcpy( getValuePtr( ), &tmp, sizeof( tmp ) );

            m_valueChanged = true;
            notifySynchronisationEvent( _latency );
        }

        /***************************************************************/
        // isValueChanged
        //!
        //! \fn       isValueChanged
        //! \brief    check for changed value
        //!
        //! \param [in] _reset if true, then set change flag to false
        //! \return     bool: true = value changed
        //!
        //! \author    Andre Werner
        //! \version   2015-6-6 : initial
        /***************************************************************/
        bool isValueChanged( bool _reset = false )
        {
            if ( m_valueChanged )
                {
                    if ( _reset )
                        resetValueChanged( );
                    return true;
                }
            else
                return false;
        }

        //! \fn resetValueChanged
        //!\brief rest value changed flag
        void resetValueChanged( ) { m_valueChanged = false; }

    private:
        // forbidden constructors
        ObserverInterconnect( ) = delete;                                     //!< \brief forbidden
        ObserverInterconnect( const ObserverInterconnect& _source ) = delete; //!< \brief forbidden
        ObserverInterconnect( ObserverInterconnect&& _source ) = delete;      //!< \brief forbidden
        ObserverInterconnect& operator=(                                      //!< \brief forbidden
            const ObserverInterconnect& _rhs ) = delete;                      //!< \brief forbidden
        ObserverInterconnect& operator=(
            ObserverInterconnect&& _rhs ) = delete; //!< \brief forbidden


    private:
        bool m_valueChanged; //!< \brief flag shows if a observed value changed
    };
}


#endif // !OBSERVERINTERCONNECT_H_
