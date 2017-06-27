
#ifndef OBSERVERMANAGER_H_
#define OBSERVERMANAGER_H_

#include "Typedefinitions.h"
#include "Observer.h"
#include "ObserverInterconnect.h"
#include <utility>
#include <iterator>
#include <map>

namespace vc_utils
{
    template < class observerTypeT > class ObserverManager
    {
        /************************************************************************/
        /* type definitions                                                     */
        /************************************************************************/
        
        //! \typedef obsPool_t 
        //! \brief definition of Observer pool
        typedef std::map< unsigned int, observerTypeT* > obsPool_t;

        //! \typedef mapsIter_t
        //! \brief type of iterator for Observer pool
        typedef typename obsPool_t::const_iterator mapsIter_t;
        

    public:
        /************************************************************************/
        /* constructor                                                          */
        /************************************************************************/

        //! \brief constructor
        ObserverManager( ) : m_obsId( 0 ) { m_observers.clear( ); }

        //! \brief destructor
        ~ObserverManager( )
        {
            for ( auto obs : m_observers )
                {
                    delete obs.second;
                }
        }

        

        /***************************************************************/
        // addObserver
        //!
        //! \func       addObserver
        //! \brief      add Observer to manager
        //!
        //! \param [in] _event module synchronization event for process notification
        //! \param [in] _data pointer to data
        //! \param [in] _length data have to be copied
        //! \return     unsigned int: observer id for the last copied element;
        //!
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        unsigned int addObserver( event_t* _event, dataPtr_t _data, unsigned int _length )
        {
            m_observers.emplace( m_obsId,
                new observerTypeT( _event, _data, _length ) );

            return m_obsId++;
        }

        //!\brief erase a observer with id _obsId from manager 
        //! \param [in] _obsId identification number of observer has to be erased
        void eraseObserver(const unsigned int _obsId ) { m_observers.erase( _obsId ); }

        //! \brief erase all observers from the manager
        void clearObservers( void )
        {
            m_observers.clear( );
            m_obsId = 0;
        }

        
        /***************************************************************/
        // getObserver
        //!
        //! \func       getObserver
        //! \brief    get a observer from manager
        //!
        //! \param [in] _obsId observer identification number (map key)
        //! \return     nullptr is Observer not found, else pointer to Observer
        //!
        //! \author    Andre Werner
        //! \version   2015-6-14 : initial
        /***************************************************************/
        observerTypeT* getObserver( unsigned int _obsId )
        {
            if ( m_observers.size( ) > _obsId )
                return m_observers.at( _obsId );
            else
                return nullptr;
        }
        
        /************************************************************************/
        /* implement functionality for range based loops                        */
        /************************************************************************/

        //! \brief return const iterator of begin of Observer pool
        mapsIter_t begin( ) { return m_observers.cbegin( ); }

        //! \brief return const iterator of end of Observer pool
        mapsIter_t end( ) { return m_observers.cend( ); }

        /************************************************************************/
        /* manager informations about data field properties                     */
        /************************************************************************/

        //! \brief return number of Observer in pool
        std::size_t getNumberOfObservers( void ) const { return m_observers.size( ); }

        //! \brief return next free automatic generated Observer id
        unsigned int getNextFreeObserverId( void ) const { return m_obsId; }

    private:
        //forbidden constructors
        ObserverManager( const ObserverManager& _source ) = delete;
        ObserverManager( ObserverManager&& _source ) = delete;
        ObserverManager& operator=( const ObserverManager& _source ) = delete;
        ObserverManager& operator=( ObserverManager&& _source ) = delete;

    private:
        /************************************************************************/
        /* member                                                               */
        /************************************************************************/
        obsPool_t m_observers; //!< \brief data field for observers (key = observer id)
        unsigned int m_obsId; //!< \brief counter to generate unique observer id
    };
}

#endif