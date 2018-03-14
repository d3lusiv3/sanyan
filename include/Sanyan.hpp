#ifndef __SANYAN_SIGSLOT_HPP
#define __SANYAN_SIGSLOT_HPP

#include <unordered_map>
#include <functional>
#include <typeinfo>

namespace sanyan
{
	//Define types to be used
	using Type_ID = size_t;

	//forward declare slot base for deffinition of slotted class
   class SlotBase;

	//inherit from this class if you want your class to contain slots
	//that can be searchd and hooked up at run-time
	class SlottedClass
	{
	   public:
		   SlottedClass() ;
         void RegisterSlot( SlotBase* slot_base );

	   private:
         std::unordered_map< std::string, SlotBase* > slots_;
	};

	//Base class for a slot
	class SlotBase
	{
	   public:
		   SlotBase( std::string slot_name, Type_ID type_ID );
         SlotBase( std::string slot_name, Type_ID type_ID, const SlottedClass* slotted_parent );
		
         std::string SlotName();
         Type_ID SlotType();
         void ReceiveBase( const void* arguments );
      

         virtual void Receive( const void*  arguments ) = 0;

      private:
		   SlotBase();
		   std::string slot_name_;
		   Type_ID type_ID_;
         const SlottedClass* slotted_parent_;
	};

   //Inherit from this class if you want to have a named slot that can be grouped
   //into a "slotted" group but you dont want to have to pass a handler function
   //for the receive callback
   template < class T >
   class InheritableSlot : public SlotBase
   {
      public:
         InheritableSlot( std::string slot_name ) : SlotBase( slot_name, typeid( T ).hash_code() ) {}

         virtual void Receive( const void* arguments ) override
         {
            T localCopy;
            memcpy( ( void* )&localCopy, arguments, sizeof( T ) );
            OnReceived( localCopy );
         }

         virtual void OnReceived( T arguments ) = 0;

      private:
         InheritableSlot( ){};

   };

   template < class T, class C >
   class MemberSlot : public SlotBase
   {
      public:
         MemberSlot( std::string slot_name, const C* callback_parent_object, void( C::*member_function_callback )( T ) ) : SlotBase( slot_name, typeid( T ).hash_code( ) ), callback_parent_object_( callback_parent_object ), member_function_callback_( member_function_callback ){}

         virtual void Receive( const void* arguments ) override
         {
            T localArgs;
            memcpy( ( void* )&localArgs, args, sizeof( T ) );
            OnReceived( localArgs );
         }

         void OnReceived( T arguments )
         {
            ( callback_parent_object_->*member_function_callback_ )( arguments );
         }

      private:
         MemberSlot() {}
         const C* callback_parent_object_;
         void( C::*member_function_callback_ )( T );
   };

	//Base Class for a signal
	class SignalBase
	{
	   public:
         SignalBase( std::string signal_name, Type_ID type_id  );

         virtual void Emit( void* arguments ) = 0;

         std::string SignalName();
         Type_ID SignalType();

      private:
         SignalBase();
         std::string  signal_name_;
         Type_ID type_ID_;
	};

   template < class T >
   class Signal : public SignalBase
   {

      public:
         Signal( std::string signal_name )
            : SignalBase( signal_name, typeid( T ).hash_code() )
         {
         }

         virtual void Emit( void* arguments ) override
         {
            for( int s = 0; s < sanyan_slots_.size( ); ++s )
            {
               sanyan_slots_[ s ]->ReceiveBase( arguments );
            }
         }

         bool Connect( SlotBase* slot_base )
         {
            if( slot_base->SlotType() == SignalType() )
            {
               sanyan_slots_.push_back( slot_base );
            }
         }

         void operator()( T arguments )
         {

            Emit( ( void* )&arguments );
         }

      private:
         Signal(){};

         std::vector < std::function< void( T ) > > non_sanyan_slots_;
         std::vector < SlotBase* > sanyan_slots_;
   };

}

#endif