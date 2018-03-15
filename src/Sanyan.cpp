#include "Sanyan.hpp"

namespace sanyan
{

   Unique_ID_Type SlotBase::slot_uuid_generator_ = 0;

	SlottedClass::SlottedClass()
	{

	}

	void
      SlottedClass::RegisterSlot( SlotBase* slot_base )
	{
      slots_[ slot_base->SlotName() ] = slot_base;
	}

   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, void( *base_function_pointer_ )( void* ) )
		:  slot_name_          ( slot_name )
		 , type_ID_            ( type_ID )
       , base_function_pointer_( base_function_pointer_ )
       , uuid_( ++slot_uuid_generator_ )
	{

	}

   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, const SlottedClass* slotted_parent )
      : slot_name_      ( slot_name )
      , type_ID_        ( type_ID )
      , slotted_parent_ ( slotted_parent )
      , uuid_( ++slot_uuid_generator_ )
   {
      
   }

   SlotBase::~SlotBase()
   {
      for( std::vector< SignalBase* >::iterator sbIT = connectedSignals_.begin( );
            sbIT != connectedSignals_.end( );
            ++sbIT )
      {
         ( *sbIT )->RemoveSlotOnDestructionBase( this );
      }
      connectedSignals_.clear();
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

   bool
   SlotBase::operator==( const SlotBase& rhs )
   {
      bool ret = false;
      if( uuid_ == rhs.UUID() )
      {
         ret = true;
      }
      return ret;
   }
   bool
      SlotBase::operator==( const SlotBase* rhs )
   {
         bool ret = false;
         if( uuid_ == rhs->UUID( ) )
         {
            ret = true;
         }
         return ret;
      }

   bool
   SlotBase::operator==( void( * const function_pointer )( void* ) )
   {
      bool ret = false;
      if( base_function_pointer_ == function_pointer )
      {
         ret = true;
      }
      return ret;
   }

   Unique_ID_Type
   SlotBase::UUID() const
   {
      return uuid_;
   }

	//private blocked default constructor
	SlotBase::SlotBase()
	{

	}

	SignalBase::SignalBase( std::string signal_name, Type_ID type_id )
		: signal_name_( signal_name )
		, type_ID_( type_id )
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