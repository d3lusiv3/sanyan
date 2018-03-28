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

	SlotBase*
	SlottedClass::GetSlot( std::string name )
	{
		SlotBase* ret = nullptr;
		ret = slots_[ name ];
		return ret;
	}

   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, void( *base_function_pointer_ )( void* ) )
		:  slot_name_          ( slot_name )
		 , type_ID_            ( type_ID )
         , base_function_pointer_( base_function_pointer_ )
         , uuid_( ++slot_uuid_generator_ )
		 , slotted_parent_( nullptr )
	{

	}

   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, SlottedClass* const slotted_parent )
      : slot_name_      ( slot_name )
      , type_ID_        ( type_ID )
      , slotted_parent_ ( slotted_parent )
      , uuid_( ++slot_uuid_generator_ )
   {
	   slotted_parent->RegisterSlot(this);
   }

   SlotBase::~SlotBase()
   {

      std::vector< SignalBase* >::iterator csIT = connectedSignals_.begin();
      while( csIT != connectedSignals_.end() )
      {
         SignalBase* signal = (*csIT );
         //erase first
         connectedSignals_.erase( csIT );
         //then call signals remove slot destruction, we do this order
         //because the signal may or may not callback on this slot with an
         //unregister signal from slot
         signal->RemoveSlotOnDestructionBase( this );
         csIT = connectedSignals_.begin();
      }
      //removeslotondestructionbase
      connectedSignals_.clear();
   }

   
   std::string
   SlotBase::SlotName() const
   {
      return slot_name_;
   }

   Type_ID
   SlotBase::SlotType() const
   {
      return type_ID_;
   }

   SlottedClass* const
   SlotBase::SlottedParent()
   {
	   return slotted_parent_;
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
		: slotted_parent_( nullptr )
	{

	}

	SignalingClass::SignalingClass()
	{

	}

	void
	SignalingClass::RegisterSignal(SignalBase* signal_base )
	{
		signals_[signal_base->SignalName()] = signal_base;
	}

	SignalBase*
	SignalingClass::GetSignal(std::string signal_name)
	{
		SignalBase* ret = nullptr;
		ret = signals_[ signal_name ];
		return ret;
	}

	SignalBase::SignalBase( std::string signal_name, Type_ID type_id )
		: signal_name_( signal_name )
		, type_ID_( type_id )
		, signaling_parent_( nullptr )
	{
	}

	SignalBase::SignalBase(std::string signal_name, Type_ID type_id, SignalingClass* const signaling_parent )
		: signal_name_(signal_name)
		, type_ID_(type_id)
		, signaling_parent_(signaling_parent )
	{
		signaling_parent_->RegisterSignal( this );
	}

   SignalBase::~SignalBase()
   {
	   for (int s = 0; s < sanyan_slots_.size(); ++s)
	   {
		   sanyan_slots_[s]->UnregisterSignalBase(this);
	   }
   }

   std::string
   SignalBase::SignalName() const
   {
      return signal_name_;
   }

   SignalingClass*
   SignalBase::SignalingParent()
   {
	  return signaling_parent_;
   }

   SignalBase::SignalBase( )
	   : signal_name_( "" )
	   , type_ID_( -1 )
	   , signaling_parent_( nullptr )
   {
   }

   Type_ID
   SignalBase::SignalType() const
   {
      return type_ID_;
   }


   bool
   CONNECT( SignalingClass* signal_class, std::string signal_name, SlottedClass* slot_class, std::string slot_name )
   {

      bool ret = false;
      if( signal_class && slot_class )
      {
         SlotBase* slot = slot_class->GetSlot( slot_name );
         SignalBase* signal = signal_class->GetSignal( signal_name );
         if( slot && signal )
         {
            ret = signal->BaseConnect( slot );
         }
         else
         {
            //TODO: throw, either slot or signal was not present
         }
      }
      else
      {
         //TODO: throw, signal or slot not valid object
      }
      
      return ret;
   }

   bool
   DISCONNECT( SignalingClass* signal_class, std::string signal_name, SlottedClass* slot_class, std::string slot_name )
   {

      bool ret = false;
      if( signal_class && slot_class )
      {
         SlotBase* slot = slot_class->GetSlot( slot_name );
         SignalBase* signal = signal_class->GetSignal( signal_name );
         if( slot && signal )
         {
            ret = signal->BaseDisconnect( slot );
         }
         else
         {
            //TODO: throw, either slot or signal was not present
         }
      }
      else
      {
         //TODO: throw, signal or slot not valid object
      }
      
      return ret;
   }

}