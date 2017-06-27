//! \file Subject.cpp
//! \brief base class of observable objects
#include "Subject.h"

namespace vc_utils
{

    unsigned int Subject::numOfSubjects = 1;

    /************************************************************************/
    /* constructor:                                                         */
    /************************************************************************/
    Subject::Subject( std::string _name ) : m_name( _name ), m_subjectID( numOfSubjects++ )
    {
        m_observerVec.clear( );
    }

    Subject::Subject( Subject&& _source ) : 
        m_name(_source.m_name), m_subjectID(_source.m_subjectID),
        m_observerVec(_source.m_observerVec)
    {

        //erase all values from source
        _source.m_name = "";
        _source.m_subjectID = UNUSED_SUBJECT;
        _source.m_observerVec.clear();
    }

    Subject& Subject::operator=( Subject&& _rhs )
    {
        this->m_name = _rhs.m_name;
        this->m_subjectID = _rhs.m_subjectID;
        this->m_observerVec = _rhs.m_observerVec;

        //erase all values from source
        _rhs.m_name = "";
        _rhs.m_subjectID = UNUSED_SUBJECT;
        _rhs.m_observerVec.clear( );

        return *this;
    }

    /************************************************************************/
    /* register Observer:                                                   */
    /************************************************************************/
    Subject& Subject::operator+=( observer_t _rhs )
    {
        this->registerObserver( _rhs.first, _rhs.second );

        return *this;
    }

    void Subject::registerObserver( observer_t _obs )
    {
        this->registerObserver( _obs.first, _obs.second );
    }

    void Subject::registerObserver( obsPtr_t _obs, unsigned int _outSocketId )
    {
        auto compareObs = [&_obs, &_outSocketId] ( observer_t& _it ) -> bool {

            return ( ( _it.first == _obs ) && ( _it.second == _outSocketId ) );
        };

        if ( std::any_of( m_observerVec.begin( ), m_observerVec.end( ), compareObs ) )
            return;
        else
            m_observerVec.push_back( observer_t( _obs, _outSocketId ) );

        return;
    }

    /************************************************************************/
    /* erase Observer:                                                      */
    /************************************************************************/
    Subject& Subject::operator-=( observer_t _rhs )
    {
        this->eraseObserver( _rhs.first, _rhs.second );

        return *this;
    }


    void Subject::eraseObserver( observer_t _obs )
    {
        this->eraseObserver( _obs.first, _obs.second );
    }


    void Subject::eraseObserver( obsPtr_t _obs, unsigned int _outSocketId )
    {
        auto compareObs = [&_obs, &_outSocketId] ( observer_t& _it ) -> bool {

            return ( ( _it.first == _obs ) && ( _it.second == _outSocketId ) );
        };

        auto place = std::find_if( m_observerVec.begin( ), m_observerVec.end( ), compareObs );

        if ( place < m_observerVec.end( ) )
            m_observerVec.erase( place );

        return;
    }


} //end of namespace vc_utils
