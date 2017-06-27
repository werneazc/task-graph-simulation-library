//! \file Typedefinitions.h
//! \brief often used typedefs, constants and header files

#ifndef TYPEDEFINITIONS_H_
#define TYPEDEFINITIONS_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#include <tlm>
#include <typeinfo>
#include <iostream>
#include <iomanip>
#include <initializer_list>

//! \brief namespace for vision computing utilities
namespace vc_utils
{


/************************************************************************/
/* type definitions:                                                    */
/************************************************************************/
typedef sc_core::sc_time sc_time_t; //!< \brief systemC time object
typedef sc_core::sc_time_unit unit_t; //!< \brief systemC time unit

typedef unsigned char* dataPtr_t; //!< \brief byte pointer
typedef sc_core::sc_event event_t; //!< \brief systemC event object

typedef sc_core::sc_module_name name_t; //!< \brief systemC module name object

typedef tlm::tlm_generic_payload trans_t; //!< \brief tlm generic payload object

/************************************************************************/
/* enumerations                                                         */
/************************************************************************/
//! \enum SIDE
//! //! \brief Side of a binary operation
enum SIDE { LHS = 0, RHS = 1 };

}

#endif