//! \file Subject.h
//! \brief base class of observable objects
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <algorithm>
#include "Observer.h"

#ifndef SUBJECT_H_
#define SUBJECT_H_

//! \def UNUSED_SUBJECT
//! \brief Zero indicates an unused subject after a move construction
#define UNUSED_SUBJECT 0

namespace vc_utils
{

    //! \class Subject
    //! \brief base class of observable objects
    class Subject
    {
    public:
        //! \typedef obsPtr_t
        //! \brief pointer which points to an Observer object
        typedef Observer* obsPtr_t;
        //! \typedef observer_t
        //! \brief std::pair with Observer pointer and his observed value (index)
        typedef std::pair< obsPtr_t, unsigned int > observer_t;

        std::vector< observer_t >
            m_observerVec; //!< vector of Observer and the index of the value they observe

    public:
        //! \brief destructor
        virtual ~Subject( ) = default;


        /***************************************************************/
        // equalization operator
        //!
        //! \brief	  	equalization operator
        //!
        //! \param [in]	_lhs	left hand side
        //! \param [in]	_rhs	right hand side
        //!
        //! \return    	true if _lhs and _rhs have the same m_subjectID
        //!
        //!	\details
        //!	This function should check if a Observer is already registered for a
        //!	out going value.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        friend bool operator==( const Subject& _lhs, const Subject& _rhs )
        {
            // check if _lhs is also _rhs by address of both
            return ( _lhs.m_subjectID == _rhs.m_subjectID );
        }


        /***************************************************************/
        // not equal operator
        //!
        //! \brief	  	not equal operator
        //!
        //! \param [in]	_lhs left hand side
        //! \param [in]	_rhs right hand side
        //!
        //! \return    	false if _lhs and _rhs have the same m_subjectID
        //!
        //!	\details
        //!	This function should check if a Observer is not already registered for a
        //!	out going value.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        friend bool operator!=( const Subject& _lhs, const Subject& _rhs )
        {
            // check if _lhs is also _rhs by address of both
            return ( _lhs.m_subjectID != _rhs.m_subjectID );
        }


        /***************************************************************/
        // operator+=:
        //!
        //! \brief	  	register Observer at Subject
        //!
        //! \param [in]	_rhs	set Observer and out going value (value by ID)
        //! \return    	Subject&:	*this
        //!
        //!	\details
        //! Register a Observer at the current Subject.
        //! The parameter is a std::pair of a pointer to the Observer
        //! and an index of the output.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        Subject& operator+=( observer_t _rhs );

        /***************************************************************/
        // operator-=:
        //!
        //! \brief	  	erase Observer at Subject
        //!
        //! \param [in]	_rhs	erase Observer for out going value (value by ID)
        //! \return    	Subject&:	*this
        //!
        //!	\details
        //! Erase a Observer at the current Subject.
        //! The parameter is a std::pair of a pointer to the Observer
        //! and an index of the output.
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        Subject& operator-=( observer_t _rhs );

        /***************************************************************/
        // registerObserver:
        //!
        //! \brief	  	register a child for a observable value
        //!
        //! \param [in]	_obs	Observer
        //! \param [in]	_outSocketId	value ID
        //!
        //!	\sa operator+=
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        void registerObserver( obsPtr_t _obs, unsigned int _outValueId );


        /***************************************************************/
        // registerObserver:
        //!
        //! \brief	  	register a child for a observable value
        //!
        //! \param [in]	_obs	std::pair of Observer and observed value ID
        //!
        //!	\sa operator+=
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        void registerObserver( observer_t _obs );


        /***************************************************************/
        // eraseObserver:
        //!
        //! \brief	  	erase child for observed value
        //!
        //! \param [in]	_obs	Observer
        //! \param [in]	_outSocketId	outgoing value id
        //!
        //!	\details
        //!	An Observer is deleted from the array of Observers for the value with id
        //!	_outValueId.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        void eraseObserver( obsPtr_t _obs, unsigned int _outValueId );


        /***************************************************************/
        // eraseObserver
        //!
        //! \brief	  	erase child for observed value
        //!
        //! \param [in]	_obs	td::pair of Observer and observed value ID
        //!
        //! \sa operator-=
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        void eraseObserver( observer_t _obs );


        /***************************************************************/
        // notifyObservers:
        //!
        //! \brief	  	Observer notification chosen by out going value ID
        //!
        //! \param [in]	_outValueId identification of observed value
        //!
        //!	\details
        //!	Notify all Observers which are sensitive for the value
        //!	specified by _outValueId.
        //!
        //!
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        virtual void notifyObservers( unsigned int _outValueId ) = 0;

        /***************************************************************/
        // getName:
        //!
        //! \brief	  	return Subject name
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        std::string getName( void ) const { return m_name; }


        /***************************************************************/
        // getName_Cstr:
        //!
        //! \brief	  	return Subject name as c-string
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        const char* getName_Cstr( void ) const { return m_name.c_str( ); }

        /***************************************************************/
        // get_subjectID:
        //!
        //! \brief	  	return Subject name as c-string
        //!	\author   	Andre Werner
        //!	\version  	2015-5-5 : initial
        /***************************************************************/
        unsigned int get_subjectID( void ) const { return m_subjectID; }

    protected:
        //! \brief constructor
        explicit Subject( std::string _name );
        Subject( ) : m_name( std::string( ) ), m_subjectID( numOfSubjects++ )
        {
            m_observerVec.clear( );
        }

        // move constructor:
        //! \brief move constructor
        Subject( Subject&& _source );
        //! \brief assignment operator for move construction
        Subject& operator=( Subject&& _rhs );

    private:
        Subject( const Subject& _source ) = delete;         //!< \brief forbidden
        Subject& operator=( const Subject& _rhs ) = delete; //!< \brief forbidden

    private:
        std::string m_name;       //!< name of Subject
        unsigned int m_subjectID; //!< unique ID of a Subject

        //! \var numOfSubjects
        //! \brief counter for unique m_subjectID
        //! \note
        //! Starts by One. A m_subjectID of Zero indicates an unused subject object.
        static unsigned int numOfSubjects;
    };



} // end of namespace vc_utils




#endif
