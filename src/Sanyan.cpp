#include "Sanyan.hpp"

namespace sanyan
{

	SlottedClass::SlottedClass()
	{

	}

	void
      SlottedClass::RegisterSlot( SlotBase* slot_base )
	{
      slots_[ slot_base->SlotName() ] = slot_base;
	}

	SlotBase::SlotBase( std::string slot_name, Type_ID type_ID )
		:  slot_name_   ( slot_name )
		 , type_ID_     ( type_ID )
	{

	}

   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, const SlottedClass* slotted_parent )
      : slot_name_      ( slot_name )
      , type_ID_        ( type_ID )
      , slotted_parent_ ( slotted_parent )
   {
      
   }

   
   std::string
   SlotBase::SlotName()
   {
      return slot_name_;
   }

   Type_ID
   SlotBase::SlotType()
   {
      return type_ID_;
   }

	void
	SlotBase::ReceiveBase( const void* arguments )
	{
      Receive( arguments );
	}

	//private blocked default constructor
	SlotBase::SlotBase()
	{

	}

	SignalBase::SignalBase( std::string signal_name, Type_ID type_id )
	{

	}

   std::string
   SignalBase::SignalName()
   {
      return signal_name_;
   }

   SignalBase::SignalBase( )
   {
   }

   Type_ID
   SignalBase::SignalType()
   {
      return type_ID_;
   }


}