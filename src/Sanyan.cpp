#include "Sanyan.hpp"

namespace sanyan
{

	SlottedClass::SlottedClass()
	{

	}

	void
		SlottedClass::RegisterSlot(Slot_Base* slot_base)
	{

	}

	SlotBase::SlotBase(std::string slot_name, Type_ID type_ID)
		:  slot_name_   ( slot_name )
		 , type_ID_     ( type_ID )
	{

	}

	void
	SlotBase::ReceiveBase(void* arguments)
	{

	}

	//private blocked default constructor
	SlotBase::SlotBase()
	{

	}

	SignalBase::SignalBase()
	{

	}
}