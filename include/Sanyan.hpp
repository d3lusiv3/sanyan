#ifndef __SANYAN_SIGSLOT_HPP
#define __SANYAN_SIGSLOT_HPP

#include <unordered_map>

namespace sanyan
{
	//Define types to be used
	using Type_ID = size_t;

	//forward declare slot base for deffinition of slotted class
	class Slot_Base;

	//inherit from this class if you want your class to contain slots
	//that can be searchd and hooked up at run-time
	class SlottedClass
	{
	public:
		SlottedClass();
		void RegisterSlot( Slot_Base* slot_base );

	private:
		std::unordered_map< std::string, Slot_Base* > slots_;
	};

	//Base class for a slot
	class SlotBase
	{
	public:
		SlotBase(std::string slot_name, Type_ID type_ID);
		void ReceiveBase(void* arguments);

	private:
		SlotBase();
		std::string slot_name_;
		Type_ID type_ID_;
	};

	//Base Class for a signal
	class SignalBase
	{
	public:
		SignalBase();
	};

}

#endif