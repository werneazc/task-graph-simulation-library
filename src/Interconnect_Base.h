//! \file Interconnect_Base.h
//! \brief Base class for intercommunication module

#ifndef INTERCONNECT_BASE_H_
#define INTERCONNECT_BASE_H_

#include "Typedefinitions.h"
#include "Subject.h"
#include "PayloadManager.h"
#include <vector>

namespace vc_utils
{

    /************************************************************************/
    /* enumerations                                                         */
    /************************************************************************/

	//! \var TARGET
	//! \brief global constant that is used to indicate traget reached
    extern const int TARGET;

//    //! \enum SOCKETID
//    //! \brief outSocketId described in directions
//    //! \details
//    //! The order of the neighbors is always the same.
    //enum SOCKETID
    //{
    //    TARGET = -1,
//        UP = 0,
//        RIGHT,
//        DOWN,
//        LEFT
//#ifdef USE_EXTENDED_NETWORK
//        ,
//        UPRIGHT,
//        UPLEFT,
//        LOWRIGHT,
//        LOWLEFT
//#endif
    //};

    //! \enum TLMCOMMSTILE
    //! \brief choose communication style for initialized data transactions
    enum TLMCOMMSTILE
    {
        NONE = -1,
        LT = 0,
        AT = 1
    };

    /************************************************************************/
    /* transmission data value structure and look up table                  */
    /************************************************************************/

    //! \struct TransmissionData
    //! \brief data struct with necessary transmission data sets
    struct TransmissionData
    {
        int outSocketId;           //!< \brief index of outgoing socket
        int relativXposition;      //!< \brief relative steps in x direction
        int relativYposition;      //!< \brief relative steps in y direction
        sc_dt::uint64 destValueId; //!< \brief identification of value at destination
    };

    //! \struct SocketIdData
    //! \brief Data struct with necessary transmission data sets
    //! \brief This is because different processing units could have different
    //! numbers of sockets and numberings of them. For that, the numbering
    //! could be set by this structure. Unused directions could be set to
    //! a not used value to generate an error or by the same values to use
    //! only one socket per direction.
    // member: left, right, up, down, upright, upleft, lowright, lowleft
    struct SocketIdData
    {
        int left;
        int right;
        int up;
        int down;
#ifdef USE_EXTENDED_NETWORK
        int upright;
        int upleft;
        int lowright;
        int lowleft;
#endif
    };


    /************************************************************************/
    /* helper functions:                                                    */
    /************************************************************************/
    /***************************************************************/
    // changeCoordinate
    //!
    //! \brief	  	change coordinate count value
    //!
    //! \param [in,out] _val value reference to value which will be changed
    //! \param [in]     _step value reference to to step size of change
    //!	\details
    //!
    //! \tparam  T type of parameters
    //!
    //!	\author   	Andre Werner
    //!	\version  	2015-6-7 : initial
    /***************************************************************/
    template < typename T = int > void changeCoordinate( T& _val, const T& _step )
    {
        // step value shoot be positive because of calculation decision
        if ( 0 > _val )
            {
                _val += sc_dt::sc_abs< T >( _step );
            }
        else
            {
                _val -= sc_dt::sc_abs< T >( _step );
            }
    }


    /**********************************************************************/
    // SocketManager
    //!
    //! \class SocketManager
    //!
    //! \brief Control outgoing data streams at a specific outgoing socket.
    //!
    //! \details
    //! A outgoing socket can only handle on connection at the same time. Therefor
    //! the manager serialize parallel connection requests and store them in a FIFO.
    //! If the socket is free, the transmission process starts at the same simulation.
    //! If not, the transmission process is suspended at notified from the SocketManager.
    //!
    //! \author Andre Werner
    //! \date Juno 2015
    /***********************************************************************/
    struct SocketManager
    {


    public:
        //!< \brief constructor
        SocketManager( ) : m_socketUsed( false ) {}

        //!< \brief destructor
        ~SocketManager( ) = default;

        //! \brief    check if socket is in use
        //!
        //! \return   true = socket used
        inline bool isSocketUsed( ) { return m_socketUsed; }

        //! \brief    set socket as used
        inline void setSocketAsUsed( ) { m_socketUsed = true; }

        //! \brief    set socket as free
        inline void setSocketAsFree( ) { m_socketUsed = false; }


        /**********************************************************************/
        // freeSocketForNextJob
        //!
        //! \brief    notify next element in socket waiting queue or set socket as free
        //!
        //! \return   true = next job, false = job queue empty
        //!
        //! \details
        //! Delta notification is used so next communication sequence at outgoing socket
        //! starts at the same simulation time.
        //!
        //! \author Andre Werner
        //!
        //! \version 2015-6-6: initial
        /***********************************************************************/
        bool freeSocketForNextJob( );

        //! \brief    save socket free synchronization event in job queue
        inline void pushBackSyncFreeSocketEv( event_t* _event )
        {
            m_socketFreeJobQueue.push_back( _event );
        }


    private:
        //! \var m_socketFreeJobQueue
        //! \brief Stores events for notification waiting communication process
        std::deque< event_t* > m_socketFreeJobQueue;

        bool m_socketUsed; //!< \brief marks a socket as free (false) or used (true)

    private:
        // forbidden constructors:
        SocketManager( const SocketManager& _rhs ) = delete;            //!< \brief forbidden
        SocketManager& operator=( const SocketManager& _rhs ) = delete; //!< \brief forbidden
        SocketManager( SocketManager&& _rhs ) = delete;                 //!< \brief forbidden
        SocketManager& operator=( SocketManager&& _rhs ) = delete;      //!< \brief forbidden
    };


    /************************************************************************/
    /* class description of Interconnect                                    */
    /************************************************************************/
    struct Interconnect_Base : public Subject
    {
        /************************************************************************/
        /* type definitions                                                     */
        /************************************************************************/
        //! \typedef obsSendDataVec
        //! \brief vector to save transaction informations for data and data length
        typedef std::vector< std::pair< dataPtr_t, unsigned int > > obsSendDataVec;

    protected:
        explicit Interconnect_Base( std::string _name, unsigned int _numOfOutSockets,
            unsigned int _numOfObs, const sc_time_t& _reqDelay, const sc_time_t& _respDelay,
            const sc_time_t& _commDelay, const sc_time_t& _routingLatency, TLMCOMMSTILE _style );

    public:
        //! \brief destructor
        virtual ~Interconnect_Base( ) = default;

    private:
        // forbidden constructor
        Interconnect_Base( ) = delete;
        Interconnect_Base( const Interconnect_Base& _source ) = delete;
        Interconnect_Base( Interconnect_Base&& _source ) = delete;
        Interconnect_Base& operator=( const Interconnect_Base& rhs ) = delete;
        Interconnect_Base& operator=( Interconnect_Base&& _rhs ) = delete;

    public:
        //! \brief set access to process unit transmission data vector
        //! \details
        //! This is because process units at the borders of a network on chip
        //! will have other communication parameters then that in the middle.
        //! The addressing parameters depend on the simulated network.
        inline void setTransactionDataVec( const std::vector< TransmissionData >& _transData )
        {
            m_transmissionDataVec = &_transData;
        }

        //! \brief set access to socket id informations depending on process unit
        //! \details
        //! This is because on some networks every processing unit could have their own
        //! number of sockets and a different numbering of them. 
        inline void setSocketIdData(const SocketIdData& _socketIdData)
        {
            m_socketId = &_socketIdData;

        }

        /***************************************************************/
        // notifyObservers
        //!
        //! \brief	  	notify all observers for value
        //!
        //! \param [in]	_outValueId	identification of return value, which observer is
        //! registered for
        //!
        //!	\details
        //! Throw an error, if value is not found.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        virtual void notifyObservers( unsigned int _outValueId ) override = 0;

    private:
        /***************************************************************/
        // getOutsocketId
        //!
        //! \brief	  	get outgoing socket id for routing tasks
        //!
        //! \param [in]	_tObjPtr	pointer to transaction object
        //! \return    	int:	outgoing socket identification for outSockets vector
        //!
        //!	\details
        //!	This function looks into the transaction extension.
        //!	If the count value of x,y direction equals zero, -1 is returned which means
        //!	target reached.
        //!	Else the new coordinates are calculated and saved into the transaction
        //!	extension and the outgoing socket id is returned.
        //!
        //! \note
        //! The implementation of the new coordinate calculation depends on the
        //! network on chip.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        virtual int getOutSocketId( trans_t* _tObjPtr ) const = 0;


    protected:
        /***************************************************************/
        // requestForOutSocket
        //!
        //! \brief	  	ask for free outgoing socket
        //!
        //! \param [in]	_event	synchronization event
        //! \param [in]	_outSocketID	identification of outgoing socket
        //! \return    	bool:	true = registered in job queue, false socket free for use
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        bool requestForOutSocket( event_t& _event, const int _outSocketID );

        /**********************************************************************/
        // getTransmissionData
        //! \brief    get transmission data set from invariant look up table specified in process
        //! unit
        //!
        //! \param [in] _obsId index of look up table for transmission data set
        //! \return   const reference to transmission data set
        //!
        //! \details
        //! The step depends on pixel per process unit (its distance between the upper
        //! left pixel value of the process unit) or counts only the steps between
        //! source and target process unit width = 1.
        //!
        //! \author Andre Werner
        //!
        //! \version 2015-6-6: initial
        /***********************************************************************/
        const TransmissionData getTransmissionData( unsigned int _obsId ) const;


        /***************************************************************/
        // packTransactionObject
        //!
        //! \brief	  	pack a transaction object
        //!
        //! \param [in]	_tObjPtr	pointer to used transaction object
        //! \param [in]	_obsId	identification of notified observer value
        //! \return    	int:	outgoing socket identification
        //!
        //!	\details
        //! The function gets all necessary data from global look-up table and
        //! observer send data vector.
        //! Then it copy all data into transaction data and returns outgoing
        //! socket id for first communication hop.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        int packTransactionObject( trans_t* _tObjPtr, const unsigned int _obsId );

        /***************************************************************/
        // getCurrenttObjPtr
        //!
        //! \brief	  	return used transaction object
        //!
        //! \return    	trans_t* const:	const pointer to used transaction object
        //!
        //!	\details
        //!	The notification method from subject needs data from the transaction object.
        //!	Therefor the pointer to the current transaction object is saved in
        //!	m_currentTObjPtr before the notification method is called.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        inline trans_t* const getCurrenttObjPtr( ) const { return m_currentTObjPtr; }

        /***************************************************************/
        // checkForValidDataPackage
        //!
        //! \brief	  	check for valid transaction object parameters
        //!
        //! \param [in]	_tObjPtr	pointer to current transaction object
        //! \return    	bool:	true = valid package, false = error
        //!
        //!	\details
        //! Not all transaction parameters are used and implemented.
        //! This methods checks for valid parameters and set the error option
        //! as status if necessary.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-6-6 : initial
        /***************************************************************/
        bool checkForValidDataPackage( trans_t* _tObjPtr );

        /************************************************************************/
        // member
        /************************************************************************/
    protected:
        sc_time_t m_requestDelay;   //!< \brief  request delay in AT stile
        sc_time_t m_responseDelay;  //!< \brief  response delay in AT stile
        sc_time_t m_commDelay;      //!< \brief  communication delay in LT stile
        sc_time_t m_routingLatency; //!< \brief  latency for routing tasks
        PayloadManager m_payloads;  //!< \brief  payload object factors
        TLMCOMMSTILE m_style;       //!< \brief choose tlm communication style for interconnect

        //! \var m_outSocketFlags
        //! \brief array of outgoing socket management flags (outgoing socket ID is index)
        std::vector< SocketManager > m_outSocketFlags;

        //! \var m_observedValTargetVec
        //! \brief saves the address and the data length of the values which has to be
        //! transmitted external
        //!
        //! \details
        //! This is used for communications of myself (process unit).
        obsSendDataVec m_observedValTargetVec;

        //! \var m_currentTObjPtr
        //! \brief  current transaction object))
        trans_t* m_currentTObjPtr;

        //! \var m_socketId
        //! \brief Socket identification numbers for routing
        const SocketIdData* m_socketId;

    private:
        //! \var m_transmissionDataVec
        //! \brief access to transmission data vector of process unit
        const std::vector< TransmissionData >* m_transmissionDataVec;
    };
}



#endif
