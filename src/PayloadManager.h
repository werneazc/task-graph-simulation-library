//! \file PayloadManager.h
//! \brief PayloadManager class declaration file


#ifndef PAYLOADMANAGER_H_
//! \def PAYLOADMANAGER_H_
//! \brief include guard
#define PAYLOADMANAGER_H_

#include <systemc>
#include <tlm>
#include <vector>
#include <string>
#include <utility>

#define EXTENSIONPOOL_UNSED

namespace vc_utils
{

    class RoutingExt;
    /*!
    * \class PayloadManager
    *
    * \brief Class manages all used transaction objects of an initiator.
    *
    * \details
    * The memory manager is used because the initialization of payload objects
    * is expensive. So if the memory manager has payload objects which aren't
    * used yet, he returns the access to one of these. Otherwise he generates a
    * new one and return the access to this one.
    *
    * \note
    * Copy and move is forbidden because of the binding of transaction
    * objects to there own memory manager. So there is only one memory
    * manager per initiator.
    *
    * The derived classes have to implement the perform functionality.
    *
    * \author Andre Werner
    *
    * \date April 2015
    *
    */
    class PayloadManager : virtual public tlm::tlm_mm_interface
    {
    private:
        std::string m_name; /*!< \brief name of the payload manager */
        std::vector< tlm::tlm_generic_payload* >
            m_payloadFreePool; /*!< \brief pool of free payload objects */
        std::vector< tlm::tlm_generic_payload* >
            m_globalPayloadPool; /*!< \brief pool of all payload objects */
        int m_poolTobjIndex;     /*!< \brief number of free payload objects */

#ifndef EXTENSIONPOOL_UNSED
        std::vector< RoutingExt* > m_globalExtPool; //!< \brief pool of all extensions
        std::vector< RoutingExt* > m_freeExtPool;   //!< \brief  pool of free extensions
        int m_poolExtIndex;        //!< number of free extensions
#endif                 


    public:
        //========================= constructor =========================
        // constructors:
        PayloadManager( );
        explicit PayloadManager( const std::string& a_name );
        virtual ~PayloadManager( );

        // forbidden constructors
        PayloadManager& operator=( const PayloadManager& a_rhs ) = delete; //!< \brief forbidden
        PayloadManager( PayloadManager&& a_param ) = delete;               //!< \brief forbidden
        PayloadManager& operator=( PayloadManager&& a_rhs ) = delete;      //!< \brief forbidden
        PayloadManager( const PayloadManager& a_param ) = delete;          //!< \brief forbidden

        //===============================================================

        //============== payload management methods =====================

        void free( tlm::tlm_generic_payload* a_tObjPtr ) override;
        tlm::tlm_generic_payload* allocate( void );

        //===============================================================

        //====================== getter and setter ======================

        unsigned int getNumberOfFreeObjects( void ) const;
        const std::string& getName( void ) const;

        //===============================================================
    };



    /************************************************************************/
    /* payload extension description                                        */
    /************************************************************************/
    /*!
     * \class RoutingExt
     *
     * \brief payload extension for routing features
     *
     * \details
     * The extension stores two relative coordinate counters which describe the
     * distance between start and goal.
     * It also has methods to write and read the values and to check if target is
     * reached easily.
     *
     * \author Andre Werner
     * \date Juno 2015
     */
    class RoutingExt : public tlm::tlm_extension< RoutingExt >
    {
    public:
        RoutingExt( );                                      //!< \brief constructor
        explicit RoutingExt( int _inital );                 //!< \brief constructor
        explicit RoutingExt( int _xInital, int _yInitial ); //!< \brief constructor
        RoutingExt( const RoutingExt& _source );            //!< \brief constructor
        RoutingExt& operator=( const RoutingExt& _rhs );    //!< \brief constructor

        virtual ~RoutingExt( ) = default; //!< \brief destructor


        // getter
        std::pair< int, int > getCoordinates( void );
        int getXCoordinate( void ) const;
        int getYCoordinate( void ) const;
        bool isTargedReached( void ) const;

        // setter
        void setXCoordinate( int _value );
        void setYCoordinate( int _value );
        void setCoordinates( int _xVal, int _yVal );
        void setCoodrinates( std::pair< int, int > _values );
        void clearCoodinates( void );

        virtual tlm_extension_base* clone( ) const override;

        virtual void copy_from( tlm_extension_base const& ext ) override;

    private:
        // forbidden constructors:
        RoutingExt( RoutingExt&& _source ) = delete;         //!< \brief forbidden
        RoutingExt& operator=( RoutingExt&& _rhs ) = delete; //!< \brief forbidden


    private:
        int m_xRefCoordinate; //!< \brief relative steps to goal in x direction
        int m_yRefCoordinate; //!< \brief relative steps to goal in y direction
    };

} // end of vc_utils




#endif // !PAYLOADMANAGER_H_
