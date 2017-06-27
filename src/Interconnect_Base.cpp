#include "Interconnect_Base.h"

const int vc_utils::TARGET = -1;

/************************************************************************/
/* SocketManger implementation                                          */
/************************************************************************/
bool vc_utils::SocketManager::freeSocketForNextJob( )
{
    if ( m_socketFreeJobQueue.empty( ) )
        {
            setSocketAsFree( );
            return false;
        }
    else
        {
            m_socketFreeJobQueue.front( )->notify( sc_core::SC_ZERO_TIME );
            m_socketFreeJobQueue.pop_front( );
            return true;
        }
}


/************************************************************************/
/* Interconnect_Base implementation                                     */
/************************************************************************/

vc_utils::Interconnect_Base::Interconnect_Base( std::string _name, unsigned int _numOfOutSockets,
    unsigned int _numOfObs, const sc_time_t& _reqDelay, const sc_time_t& _respDelay,
    const sc_time_t& _commDelay, const sc_time_t& _routingLatency, TLMCOMMSTILE _style )
    : vc_utils::Subject( _name ),
      m_observedValTargetVec( _numOfObs ),
      m_outSocketFlags( _numOfOutSockets ),
      m_currentTObjPtr( nullptr ),
      m_transmissionDataVec( nullptr ),
      m_requestDelay( _reqDelay ),
      m_responseDelay( _respDelay ),
      m_commDelay( _commDelay ),
      m_routingLatency( _routingLatency ),
      m_payloads( _name + "_MemoryManager" ),
      m_style( _style )
{
}


bool vc_utils::Interconnect_Base::requestForOutSocket( event_t& _event, const int _outSocketID )
{
    if ( m_outSocketFlags[ _outSocketID ].isSocketUsed( ) )
        {
            m_outSocketFlags[ _outSocketID ].pushBackSyncFreeSocketEv( &_event );
            return true;
        }
    else
        {
            m_outSocketFlags[ _outSocketID ].setSocketAsUsed( );

            return false;
        }
}

const vc_utils::TransmissionData vc_utils::Interconnect_Base::getTransmissionData(
    unsigned int _obsId ) const
{
    if ( m_transmissionDataVec->size( ) < _obsId )
        SC_REPORT_ERROR(
            "TransmissionDataVec", "no valid observer address for transmission data set." );

    return m_transmissionDataVec->at(_obsId);
}

int vc_utils::Interconnect_Base::packTransactionObject(
    trans_t* _tObjPtr, const unsigned int _obsId )
{
    if ( m_transmissionDataVec == nullptr )
        SC_REPORT_ERROR( this->getName_Cstr( ), "transmission data vector not initialized" );

    // look for transaction data
    auto transmissionParameter = getTransmissionData( _obsId );
    auto dataParameter = m_observedValTargetVec[ _obsId ];

    // past values into
    _tObjPtr->set_data_length( dataParameter.second );
    _tObjPtr->set_data_ptr( dataParameter.first );

    _tObjPtr->set_command( tlm::TLM_READ_COMMAND );
    _tObjPtr->set_address( transmissionParameter.destValueId );
    _tObjPtr->set_streaming_width( dataParameter.second );

    // copy data into extension
    auto tmpExt = _tObjPtr->get_extension< RoutingExt >( );

    if ( ( tmpExt == nullptr ) || ( tmpExt == NULL ) )
        SC_REPORT_ERROR( this->getName_Cstr( ), "extension is not available!" );

    tmpExt->setCoordinates(
        transmissionParameter.relativXposition, transmissionParameter.relativYposition );

    // return outgoing socket ID
    return transmissionParameter.outSocketId;
}

bool vc_utils::Interconnect_Base::checkForValidDataPackage( trans_t* _tObjPtr )
{
    auto dataLength = _tObjPtr->get_data_length( );
    auto byteEnablePtr = _tObjPtr->get_byte_enable_ptr( );
    // auto byteLength = _tObjPtr->get_byte_enable_length();
    // auto addr = _tObjPtr->get_address();
    // auto command = _tObjPtr->get_command();
    auto streamingWidth = _tObjPtr->get_streaming_width( );

    if ( dataLength > streamingWidth )
        {
            SC_REPORT_INFO( this->getName_Cstr( ), "streaming with not implemented" );
            _tObjPtr->set_response_status( tlm::TLM_GENERIC_ERROR_RESPONSE );
            return false;
        }


    if ( byteEnablePtr != NULL )
        {
            SC_REPORT_INFO( this->getName_Cstr( ), "byte enable not implemented" );
            _tObjPtr->set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
            return false;
        }

    _tObjPtr->set_response_status( tlm::TLM_OK_RESPONSE );
    return true;
}
