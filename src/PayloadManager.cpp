//! \file PayloadManager.cpp
//! \brief PayloadManager class implementation file


#include "PayloadManager.h"
namespace vc_utils
{
    //========================= constructor =========================

    /***************************************************************/
    //! \brief	  	default constructor
    //!
    //! \return    	payload manager object
    //!
    //!	\details   	Calls explicit constructor with standard string.
    //!
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    PayloadManager::PayloadManager( ) : PayloadManager( "defaultPayloadManager" ) {}

    /***************************************************************/
    //! \brief	  	Explicit constructor
    //!
    //! \param [in]	a_name	name of payload manager
    //! \return    	payload manager object
    //!
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    PayloadManager::PayloadManager( const std::string& a_name )
        : m_name( a_name ), m_poolTobjIndex( -1 )
    {
        // generate empty payload pool
        m_payloadFreePool.clear( );
        m_globalPayloadPool.clear( );
    }

    /***************************************************************/
    //! \brief	  	Destructor
    //!
    //!	\details
    //!	Destructor generates an exception if the reference counter of
    //!	payload objects are not zero.
    //!
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    PayloadManager::~PayloadManager( )
    {
        for ( auto it : m_globalPayloadPool )
            {
                if ( it->get_ref_count( ) )
                    SC_REPORT_INFO( m_name.c_str( ), "Still payload objects inn use." );
                delete it;
            }
    }

    //===============================================================

    //====================== getter and setter ======================

    /***************************************************************/
    //! \brief	  	return name of payload manager
    //!
    //! \return    	const std::string&:	name of payload manager
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    const std::string& PayloadManager::getName( void ) const { return m_name; }

    /***************************************************************/
    //! \brief	  	return number of free payload objects in payload pool
    //!
    //! \return    	unsigned int:	number of free payload objects
    //!
    //!	\details
    //!	A negative value of m_poolFreeIndex means, that no free payload
    //!	objects available. So the number of free payload objects is zero
    //!	or the value of the variable.
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    unsigned int PayloadManager::getNumberOfFreeObjects( void ) const
    {
        return ( 0 > m_poolTobjIndex ) ? 0 : m_poolTobjIndex;
    }

    //===============================================================

    //============== payload management methods =====================

    /***************************************************************/
    //! \brief	  	Get access to free payload object
    //!
    //! \return    	tlm::tlm_generic_payload*:	access to free payload
    // object
    //!
    //!	\details
    //! As long as the reference counter of the object is bigger than zero,
    //! the payload object could be used exclusively.
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    tlm::tlm_generic_payload* PayloadManager::allocate( void )
    {
        tlm::tlm_generic_payload* t_tObjPtr = nullptr;
        if ( 0 > m_poolTobjIndex )
            {
                // generate new transport object and define it as memory managed
                // by this (payloadManager)
                t_tObjPtr = new tlm::tlm_generic_payload( this );
                m_globalPayloadPool.push_back( t_tObjPtr );
            }
        else
            {
                // use free transport object and set pool index to next free
                // object in pool
                t_tObjPtr = m_payloadFreePool[ m_poolTobjIndex ];
                m_payloadFreePool[ m_poolTobjIndex-- ] = nullptr;
            }

#ifndef EXTENSIONPOOL_UNSED
        // could not save extension objects in vector compiler error!
        RoutingExt* tmpExt = nullptr;
        if ( 0 > m_poolExtIndex )
            {
                tmpExt = new RoutingExt( 0 );
                m_globalExtPool.push_back( tmpExt );
            }
        else
            {
                tmpExt = m_freeExtPool[ m_poolExtIndex ];
                m_freeExtPool[ m_poolExtIndex-- ] = nullptr;
            }


        // put extension into transaction object
        t_tObjPtr->set_extension< RoutingExt >(tmpExt);
#endif
#ifdef EXTENSIONPOOL_UNSED

        //alternative
        RoutingExt* tmpExt = new RoutingExt(0);
        t_tObjPtr->set_auto_extension< RoutingExt >(tmpExt);
#endif



        return t_tObjPtr;
    }

    /***************************************************************/
    //! \brief	  	Manage free payload object pool
    //!
    //! \param [in]	a_tObjPtr	payload object should be placed in
    //!                         payload  pool
    //!
    //!	\details
    //! If reference count of a payload object is zero, these method is called
    //! automatically. If the pool vector is empty or to short, it is increased,
    //! else the payload object pointer is placed on the next free location.
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-4-17 : initial
    /***************************************************************/
    void PayloadManager::free( tlm::tlm_generic_payload* a_tObjPtr )
    {
        // reset gp attributes
        a_tObjPtr->set_command( tlm::TLM_IGNORE_COMMAND );
        a_tObjPtr->set_address( 0 );
        a_tObjPtr->set_data_ptr( NULL );
        a_tObjPtr->set_data_length( 0 );
        a_tObjPtr->set_byte_enable_ptr( 0 );
        a_tObjPtr->set_byte_enable_length( 0 );
        a_tObjPtr->set_streaming_width( 0 );
        a_tObjPtr->set_dmi_allowed( false );
        a_tObjPtr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

        m_poolTobjIndex++;
        // check if dynamic payload pool is to small and increase its size if
        // necessary
        if ( ( m_payloadFreePool.empty( ) ) || ( m_poolTobjIndex > m_payloadFreePool.size( ) - 1 ) )
            m_payloadFreePool.push_back( a_tObjPtr );
        else
            m_payloadFreePool[ m_poolTobjIndex ] = a_tObjPtr;


#ifndef EXTENSIONPOOL_UNSED
        m_poolExtIndex++;
        if ( ( m_freeExtPool.empty( ) ) || ( m_poolExtIndex > m_freeExtPool.size( ) - 1 ) )
            m_freeExtPool.push_back( a_tObjPtr->get_extension< RoutingExt >( ) );
        else
            m_freeExtPool[ m_poolExtIndex ] = a_tObjPtr->get_extension< RoutingExt >( );
#endif
    }

    //===============================================================

    /************************************************************************/
    /* payload extension implementation                                     */
    /************************************************************************/

    /************************************************************************/
    /* constructor, destructor                                              */
    /************************************************************************/

    RoutingExt::RoutingExt( ) : m_xRefCoordinate( 0 ), m_yRefCoordinate( 0 ) {}

    RoutingExt::RoutingExt( int _inital ) : m_xRefCoordinate( _inital ), m_yRefCoordinate( _inital )
    {
    }

    RoutingExt::RoutingExt( int _xInital, int _yInitial )
        : m_xRefCoordinate( _xInital ), m_yRefCoordinate( _yInitial )
    {
    }

    RoutingExt::RoutingExt( const RoutingExt& _source )
    {
        m_xRefCoordinate = _source.getXCoordinate( );
        m_yRefCoordinate = _source.getYCoordinate( );
    }

    RoutingExt& RoutingExt::operator=( const RoutingExt& _rhs )
    {
        this->m_xRefCoordinate = _rhs.getXCoordinate( );
        this->m_yRefCoordinate = _rhs.getYCoordinate( );
        return *this;
    }



    /***************************************************************/
    // clone
    //!
    //! \brief	  copy of extension
    //!
    //! \return    	tlm::tlm_extension_base*:	pointer to copy on extension
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-6-6 : initial
    /***************************************************************/
    tlm::tlm_extension_base* RoutingExt::clone( ) const
    {
        RoutingExt* tmpExtension = new RoutingExt( *this );
        return tmpExtension;
    }

    /***************************************************************/
    // copy_from
    //!
    //! \brief	  	copy from extension
    //!
    //! \param [in]	ext	copy source extension
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-6-6 : initial
    /***************************************************************/
    void RoutingExt::copy_from( tlm::tlm_extension_base const& ext )
    {
        *this = dynamic_cast< const RoutingExt& >( ext );
    }

    //! \brief return relative steps of coordinates (x, y)
    std::pair< int, int > RoutingExt::getCoordinates( )
    {
        return std::make_pair( m_xRefCoordinate, m_yRefCoordinate );
    }

    //! \brief return relative steps of x coordinate
    int RoutingExt::getXCoordinate( ) const { return m_xRefCoordinate; }

    //! \brief return relative steps of y coordinate
    int RoutingExt::getYCoordinate( ) const { return m_yRefCoordinate; }

    //! \brief set relative steps of x coordinate
    void RoutingExt::setXCoordinate( int _value ) { m_xRefCoordinate = _value; }

    //! \brief set relative steps of y coordinate
    void RoutingExt::setYCoordinate( int _value ) { m_yRefCoordinate = _value; }

    //! \brief set relative steps of coordinates (x, y)
    void RoutingExt::setCoordinates( int _xVal, int _yVal )
    {
        m_xRefCoordinate = _xVal;
        m_yRefCoordinate = _yVal;
    }

    //! \brief set relative steps of coordinates (x, y)
    void RoutingExt::setCoodrinates( std::pair< int, int > _values )
    {
        m_xRefCoordinate = _values.first;
        m_yRefCoordinate = _values.second;
    }

    //! \brief set relative coordinates to zero
    void RoutingExt::clearCoodinates( )
    {
        m_xRefCoordinate = 0;
        m_yRefCoordinate = 0;
    }


    /***************************************************************/
    // isTargedReached
    //!
    //! \brief	  	check if target process unit is reached
    //!
    //! \return    	bool:	true = target is reached
    //!
    //!	\details   	
    //!	If both relative coordinate counters are zero, the target is
    //!	reached and the function returns true.
    //!
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-6-6 : initial
    /***************************************************************/
    bool RoutingExt::isTargedReached( void ) const
    {
        if ( ( m_xRefCoordinate == 0 ) && ( m_yRefCoordinate == 0 ) )
            return true;
        else
            return false;
    }


}