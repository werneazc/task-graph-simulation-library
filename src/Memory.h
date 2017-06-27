//! \file Memory.h
//! \brief universal memory module for task graph simulator

#ifndef MEMORY_H_
#define MEMORY_H_


#include "Typedefinitions.h"
#include "Subject.h"
#include "ObserverManager.h"
#include <map>
#include <array>
#include <utility>
#include <memory>

namespace vc_utils
{

	enum class TYPE : short {
		CHAR,
		SIGNED_CHAR,
		UNSIGNED_CHAR,
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		LONG,
		UNSIGNED_LONG,
		LONG_LONG,
		UNSIGNED_LONG_LONG,
		FLOAT,
		DOUBLE,
		LONG_DOUBLE
	};


	//! \struct MemoryValueBase
	//! \brief base class to save arbitrary values at memory
	struct MemoryValueBase
	{
		MemoryValueBase(std::string _name, unsigned int _valueId, unsigned int _length, TYPE _datatype)
			: m_valueId(_valueId), m_length(_length), m_dataType(_datatype), m_name(_name) {}

		virtual ~MemoryValueBase() = default;

	public:
		TYPE m_dataType;
        std::string m_name;
		unsigned int m_valueId;
		unsigned int m_length;
	};


	//! \struct MemoryValue
	//! \brief arbitrary value representation at memory
	//! \Tparam T data type of saved value
	//! \details
	//! Values requested from memory by a byte pointer and a data length.
	template <typename T>
	struct MemoryValue : public MemoryValueBase
	{
	public:
		//! \typedef value_type
		//! \brief value type of specific value
		typedef typename T value_type;

	public:
		//! \brief constructor
		MemoryValue(const value_type& _value, std::string _name, unsigned int _valueId, TYPE _dataType) :
			MemoryValueBase(_name, _valueId, sizeof(value_type), _dataType),
			m_value(_value)
		{}

		//! \brief empty values are forbidden
		MemoryValue() = delete;

		//! \brief destructor
		virtual ~MemoryValue() = default;

	public:
		//! \brief specific value
		T m_value;
	};


	class Memory : public Subject, public sc_core::sc_module
	{
	public:
		/************************************************************************/
		//!  \brief constructor
		//!
		//!  \param [in] _name     name of Memory object
		//!
		/************************************************************************/
		SC_HAS_PROCESS(Memory);
		Memory(name_t _name)
			: sc_core::sc_module(_name),
			Subject(std::string(_name))
		{

			// generate static simulation thread
			SC_THREAD(notifyForGeneratedOutPix);
			SC_THREAD(NotifyAllCurrentValues);

		}

		//! \brief destructor
		virtual ~Memory() = default;

		// forbidden constructors
	private:
		Memory() = delete;                               //!< \brief forbidden constructor
		Memory(const Memory& _source) = delete;         //!< \brief forbidden constructor
		Memory(Memory&& _source) = delete;              //!< \brief forbidden constructor
		Memory& operator=(const Memory& _rhs) = delete; //!< \brief forbidden constructor
		Memory& operator=(Memory&& _rhs) = delete;      //!< \brief forbidden constructor

	public:
		/************************************************************************/
		/* Observers                                                            */
		/************************************************************************/
		//! \var inputObs
		//! \brief ObserverManager for Memory object
		//! \details
		//! Used to print generated values after the algorithm is done.
		ObserverManager< Observer > inputObs;


	public:
		/************************************************************************/
		// notifyObservers
		//!
		//! \brief notify successor Observers
		//!
		//! \details
		//! The _outValueId specifies the Observers which are notified.
		//!
		//! \param [in] _outValueId value identification number
		/************************************************************************/
		virtual void notifyObservers(unsigned int _outValueId) override;

		/************************************************************************/
		// addMemoryValue
		//!
		//! \brief print algorithm results
		//!
		//!	\param [in] _value		new value to save in memory
		//! \param [in] _id	value identification number
		//!
		//! \details
		//! After all algorithm results are generated, they are printed on std.
		//! output.
		//!
		//! \Tparam data type of new value
		/************************************************************************/
		template <typename T>
		void addMemoryValue(const T& _value, std::string _name, unsigned int _id, TYPE _dataType, bool _observed = false)
		{
			auto tmpValue = new MemoryValue<T>(_value, _name, _id, _dataType);

			//save data pointer to value and data length
			auto dataPtr = reinterpret_cast<vc_utils::dataPtr_t>(&tmpValue->m_value);
			unsigned int length = tmpValue->m_length;


			if (_observed)
			{
				this->m_putPixelEv.emplace_back(std::unique_ptr<event_t>(new event_t((this->getName() + "_obsEvent_" + std::to_string(_id)).c_str())));
				m_outPixEvAndList &= *m_putPixelEv.back().get();
				auto obsId = this->inputObs.addObserver(m_putPixelEv.back().get(), dataPtr, length);
				m_observerIdmap.insert(std::make_pair(_id, obsId));

				m_outputValueMap.insert(std::make_pair(_id, std::unique_ptr<MemoryValueBase>(tmpValue)));
				
			} 
			else
			{
				//value id at memory has to be unique!
				//save value information for observer notification (value address and data length)
				auto retVal = m_valueInfoMap.insert(std::make_pair(_id, std::make_pair(dataPtr, length)));
				
				//if value id already exists, the simulation crashes
				if (!retVal.second)
					SC_REPORT_ERROR(this->getName_Cstr(), "value identification wasn't unique. Value not added into memory");
	
				// specific values are saved in memory vec
				m_MemoryValueMap.insert(std::make_pair(_id, std::unique_ptr<MemoryValueBase>(tmpValue)));
			}

			return;
		}

		/************************************************************************/
		// changeMemoryValue
		//!
		//! \brief print algorithm results
		//!
		//! \details
		//! After all algorithm results are generated, they are printed on std.
		//! output.
		/************************************************************************/
		template <typename T>
		void changeMemoryValue(const T& _value, unsigned int _valueId)
		{
			//check if value under address does exists
			if (!m_MemoryValueMap.count(_valueId))
				SC_REPORT_ERROR(this->getName_Cstr(), "value identification not found at memory");


			switch (m_MemoryValueMap[_valueId]->m_dataType)
			{
			case TYPE::CHAR:
			{
				auto valuePtr = static_cast<MemoryValue<char>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::SIGNED_CHAR:
			{
				auto valuePtr = static_cast<MemoryValue<signed char>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::UNSIGNED_CHAR:
			{
				auto valuePtr = static_cast<MemoryValue<unsigned char>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::SHORT:
			{
				auto valuePtr = static_cast<MemoryValue<short>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::UNSIGNED_SHORT:
			{
				auto valuePtr = static_cast<MemoryValue<unsigned short>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::INT:
			{
				auto valuePtr = static_cast<MemoryValue<int>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::UNSIGNED_INT:
			{
				auto valuePtr = static_cast<MemoryValue<unsigned int>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::LONG:
			{
				auto valuePtr = static_cast<MemoryValue<long>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::UNSIGNED_LONG:
			{
				auto valuePtr = static_cast<MemoryValue<unsigned long>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::LONG_LONG:
			{
				auto valuePtr = static_cast<MemoryValue<long long>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::UNSIGNED_LONG_LONG:
			{
				auto valuePtr = static_cast<MemoryValue<unsigned long long>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::DOUBLE:
			{
				auto valuePtr = static_cast<MemoryValue<double>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::LONG_DOUBLE:
			{
				auto valuePtr = static_cast<MemoryValue<long double>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			case TYPE::FLOAT:
			{
				auto valuePtr = static_cast<MemoryValue<float>*>(m_MemoryValueMap.at(_valueId).get());
				valuePtr->m_value = _value;
				break;
			}
			default:
				SC_REPORT_ERROR(this->getName_Cstr(), "no valid data type found");
				break;
			};
		
			return;
		}


	public:
		/************************************************************************/
		// notifyForGeneratedOutPix
		//!
		//! \brief print algorithm results
		//!
		//! \details
		//! After all algorithm results are generated, they are printed on std.
		//! output.
		/************************************************************************/
		void notifyForGeneratedOutPix(void);

	public:
		/************************************************************************/
		// NotifyAllCurrentValues
		//!
		//! \brief start SystemC simulation by notifying all current data necessary for algorithm
		//!
		/************************************************************************/
		void NotifyAllCurrentValues(void);

		//! \brief get observer id of Observer Manager by entering own observer id.
		//! \note wrong index throw out of range exception.
		unsigned int operator[](const unsigned int& _obsId)
		{
			return m_observerIdmap.at(_obsId);
		}

	public:
		//! \brief return kind of SystemC module as string
		virtual const char* kind() const override { return "Memory"; }

		//! \brief return kind of SystemC module as string
		virtual void print(::std::ostream& os = ::std::cout) const override
		{
			os << this->name();
			return;
		}

		//! \brief return name and kind of SystemC module as string
		virtual void dump(::std::ostream& os = ::std::cout) const override
		{
			os << this->name() << ", " << this->kind();
			return;
		}

		//! \brief print values of all result pixels (m_outPixels)
		void dumpOutPixel(std::ostream& _os = ::std::cout) const;

	private:
		//! \var m_valueInfoMap
		//! \brief BRIEF
		std::map<unsigned int, std::pair<vc_utils::dataPtr_t, unsigned int> > m_valueInfoMap;
		//! \var m_MemoryValueMap
		//! \brief BRIEF
		std::map<unsigned int, std::unique_ptr<MemoryValueBase> > m_MemoryValueMap;

		//! \var m_observerIdmap
		//! \brief BRIEF
		std::map<unsigned int, unsigned int> m_observerIdmap;
		//! \var m_outputValueMap
		//! \brief return values after the algorithm is finished
		std::map< unsigned int, std::unique_ptr<MemoryValueBase> > m_outputValueMap;
		//! \var m_putPixelEv
		//! \brief SystemC events to synchronize print of algorithm results
		std::vector< std::unique_ptr<event_t> > m_putPixelEv;
		//! \var m_outPixEvAndList
		//! \brief SystemC event and list to synchronize memory output process
		sc_core::sc_event_and_list m_outPixEvAndList;
	};
};


#endif // !MEMORY_H_
