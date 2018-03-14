#ifndef __SANYAN_SIGSLOT_HPP
#define __SANYAN_SIGSLOT_HPP


#include <unordered_map>
#include <functional>
#include <typeinfo>

#ifdef _DEBUG
#ifdef _SANYAN_EXTRA_DEBUG_INFO
#include <iostream>
#include <string>
#define SANYAN_DEBUG_INFO( msg ) std::cout << "SANYAN DEBUG INFO: " << msg << std::endl
#else
#define SANYAN_DEBUG_INFO( msg )
#endif
#elif
#define SANYAN_DEBUG_INFO( msg )
#endif

namespace sanyan
{



	//Define types to be used
	using Type_ID = size_t;
	using Function_ID_Type = intptr_t;

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
		   SlotBase( std::string slot_name, Type_ID type_ID, Function_ID_Type receive_function_id );
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
		   Function_ID_Type receive_function_id_;
	};

   //Inherit from this class if you want to have a named slot that can be grouped
   //into a "slotted" group but you dont want to have to pass a handler function
   //for the receive callback
   template < class T >
   class InheritableSlot : public SlotBase
   {
      public:
		  InheritableSlot(std::string slot_name) : SlotBase(slot_name, typeid(T).hash_code(), (intptr_t)&InheritableSlot::OnReceived) {}

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

   //TODO: this needs a better name I think
   template < class T >
   class FunctinalSlot : public SlotBase
   {
	 public:
		 FunctinalSlot( std::string slot_name, void( *function_pointer )( T ) ) : SlotBase( slot_name, typeid( T ).hash_code() ), function_pointer_( function_pointer ){}

		 virtual void Receive( const void* arguments ) override
		 {
			 T localCopy;
			 memcpy( (void*)&localCopy, arguments, sizeof( T ) );
			 (*function_pointer_)( localCopy );
		 }

		 bool operator==( void( *rhs )( T ) )
		 {
			 bool ret = false;
			 if ( function_pointer_ == rhs )
			 {
				 ret = true;
			 }
			 return ret;
		 }

	  private:
		  FunctinalSlot(){}
		  void( *function_pointer_ )( T );
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
			else
			{
				//TODO: Throw an error about type mismatch
			}
         }

		 bool Connect( void( function_pointer )( T ) )
		 {
			 bool ret = false;

			 SANYAN_DEBUG_INFO("HASHES: " << typeid(T).hash_code() << " :: " << SignalType() << std::endl);
			 if( typeid( T ).hash_code() == SignalType() )
			 {
				 //we create a nameless functional slot here
				 //TODO: also create a connect that allows to pass
				 //in a function pointer and a name for the slot
				 sanyan_slots_.push_back(new FunctinalSlot< T >( "", function_pointer ) );
				 ret = true;
			 }
			 else
			 {
				 //TODO: Throw an error about type mismatch
				 ret = false;
			 }
			 return ret;
		 }

		 bool Disconnect( void( function_pointer  )( T ) )
		 {
			 bool ret = false;

			 for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin();
				 ssIT != sanyan_slots_.end();
				 ++ssIT)
			 {
				 if( (*ssIT) == function_pointer )
				 {
					 sanyan_slots_.erase( ssIT );
					 ret = true;
					 break;
				 }
			 }
		 }

         void operator()( T arguments )
         {

            Emit( ( void* )&arguments );
         }

      private:
         Signal(){};

         std::vector < SlotBase* > sanyan_slots_;
   };

}

#endif