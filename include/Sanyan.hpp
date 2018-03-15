#ifndef __SANYAN_SIGSLOT_HPP
#define __SANYAN_SIGSLOT_HPP


//#define _SANYAN_EXTRA_DEBUG_INFO

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

#define MEMBERSLOT( name, type, parentClass )  void name( type ); \
sanyan::MemberSlot< type,  parentClass > name##Slot = { #name, this, &parentClass::name }

#define MEMBERSIGNAL( name, type, parentClass ) sanyan::Signal< type > name = { #name, this }

namespace sanyan
{



	//Define types to be used
	using Type_ID = size_t;
   using Unique_ID_Type = unsigned long;

	//forward declare slot base for deffinition of slotted class
   class SlotBase;
   class SignalBase;
	//inherit from this class if you want your class to contain slots
	//that can be searchd and hooked up at run-time
	class SlottedClass
	{
	   public:
		   SlottedClass();
         void RegisterSlot( SlotBase* slot_base );

	   private:
         std::unordered_map< std::string, SlotBase* > slots_;
	};

   class SignalToSlotPrivateInterface
   {
      
      private:
         friend class SignalBase;
         void RegisterSignalBase( SignalBase* signal ) { RegisterSignal( signal ); }
         virtual void RegisterSignal( SignalBase* signal ) = 0;
         void UnregisterSignalBase( SignalBase* signal ) { UnregisterSignal( signal ); }
         virtual void UnregisterSignal( SignalBase* signal ) = 0;
   };

   class SlotToSignalPrivateInterface
   {
      private:
         friend class SlotBase;
         void RemoveSlotOnDestructionBase( SlotBase* slot_base ) { RemoveSlotOnDestruction( slot_base ); }
         virtual void RemoveSlotOnDestruction( SlotBase* slot_base ) = 0;
   };

	//Base class for a slot
   class SlotBase : public SignalToSlotPrivateInterface
	{
	   public:
           SlotBase( std::string slot_name, Type_ID type_ID, void( *base_function_pointer_ )( void* ) );
           SlotBase( std::string slot_name, Type_ID type_ID, SlottedClass* const slotted_parent );

           virtual ~SlotBase();
           std::string SlotName();
           Type_ID SlotType();
           void ReceiveBase( const void* arguments );
           Unique_ID_Type UUID() const;
           
           bool operator==( const SlotBase& rhs );
           bool operator==( const SlotBase* rhs );
           bool operator==( void( * const function_pointer )( void* ) );

           virtual void Receive( const void*  arguments ) = 0;
		   SlottedClass* const SlottedParent();
           
      private:

         virtual void RegisterSignal( SignalBase* signal ) override { connectedSignals_.push_back( signal ); }
         virtual void UnregisterSignal( SignalBase* signal ) override 
         {
            for( std::vector< SignalBase* >::iterator sbIT = connectedSignals_.begin();
                 sbIT != connectedSignals_.end();
                 ++sbIT )
            {
               if( ( *sbIT ) == signal )
               {
                  connectedSignals_.erase( sbIT );
                  break;
               }
            }
         }

		 SlotBase();
	     std::string slot_name_;
         Type_ID type_ID_;
	     SlottedClass* const slotted_parent_;
         void( *base_function_pointer_ )( void* );
         static Unique_ID_Type slot_uuid_generator_;
         Unique_ID_Type uuid_;
         std::vector< SignalBase* > connectedSignals_;
	};

   //Inherit from this class if you want to have a named slot that can be grouped
   //into a "slotted" group but you dont want to have to pass a handler function
   //for the receive callback
   template < class T >
   class InheritableSlot : public SlotBase
   {
      public:
         InheritableSlot( std::string slot_name ) : SlotBase( slot_name, typeid( T ).hash_code( ),  (void( * )( void* ))nullptr ) {}

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
       FunctinalSlot( std::string slot_name, void( *function_pointer )( T ) ) : SlotBase( slot_name, typeid( T ).hash_code( ), ( void( *)( void* ) )function_pointer ), function_pointer_( function_pointer ) {}

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
		  MemberSlot(std::string slot_name, C* const callback_parent_object, void(C::*member_function_callback)(T)) : SlotBase(slot_name, typeid(T).hash_code(), callback_parent_object), callback_parent_object_(callback_parent_object), member_function_callback_(member_function_callback){};

         virtual void Receive( const void* arguments ) override
         {
            T localArgs;
			memcpy((void*)&localArgs, arguments, sizeof(T));
            OnReceived( localArgs );
         }

         void OnReceived( T arguments )
         {
            ( callback_parent_object_->*member_function_callback_ )( arguments );
         }

      private:
         MemberSlot() {}
         C* const callback_parent_object_;
         void( C::*member_function_callback_ )( T );
   };

   class SignalingClass
   {
      public:
		  SignalingClass();
		  void RegisterSignal(SignalBase* signal_base);
      private:
		  std::unordered_map< std::string, SignalBase* > signals_;

   };

	//Base Class for a signal
   class SignalBase : public SlotToSignalPrivateInterface
	{
	   public:
         SignalBase( std::string signal_name, Type_ID type_id  );
		 SignalBase( std::string signal_name, Type_ID type_id, SignalingClass* const signaling_parent );
		 virtual ~SignalBase();

		 std::string SignalName();
		 SignalingClass* const SignalingParent();

      protected:

         void BaseEmit( void* arguments )
         {
            for( int s = 0; s < sanyan_slots_.size( ); ++s )
            {
               sanyan_slots_[ s ]->ReceiveBase( arguments );
            }
         }

         bool BaseConnect( SlotBase* slot_base )
         {
            bool ret = false;
            if( slot_base->SlotType( ) == SignalType( ) )
            {
               ret = true;
               sanyan_slots_.push_back( slot_base );
               slot_base->RegisterSignalBase( this );
            }
            else
            {
               //TODO: Throw an error about type mismatch
            }
            return ret;
         }

         bool BaseConnect( SlotBase& slot_base )
         {
            bool ret = false;
            if( slot_base.SlotType( ) == SignalType( ) )
            {
               sanyan_slots_.push_back( &slot_base );
               slot_base.RegisterSignalBase( this );
               ret = true;
            }
            else
            {
               //TODO: Throw an error about type mismatch
            }
            return ret;
         }

         bool BaseDisconnect( void( *function_pointer )( void* )  )
         {
            bool ret = false;
            for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin( );
                 ssIT != sanyan_slots_.end( );
                 ++ssIT )
            {
               if( *( *ssIT ) == function_pointer )
               {
                  sanyan_slots_.erase( ssIT );
                  ret = true;
                  break;
               }
            }

            return ret;
         }

         bool BaseDisconnect( SlotBase* slot_base )
         {
            bool ret = false;
            for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin( );
                 ssIT != sanyan_slots_.end( );
                 ++ssIT )
            {
               if( *( *ssIT ) == slot_base )
               {
                  sanyan_slots_.erase( ssIT );
                  ret = true;
                  break;
               }
            }

            return ret;
         }

         
         Type_ID SignalType();

         protected:
         

      private:
         SignalBase();

         virtual void RemoveSlotOnDestruction( SlotBase* slot_base ) override
         {
            BaseDisconnect( slot_base );
         }

         std::vector < SlotBase* > sanyan_slots_;
         std::string  signal_name_;
		 SignalingClass* const signaling_parent_;
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

		 Signal(std::string signal_name, SignalingClass* const signaling_parent)
			 : SignalBase(signal_name, typeid(T).hash_code(), signaling_parent)
		 {
		 }

         bool Connect( SlotBase& slot_base )
         {
            return BaseConnect( slot_base );
         }

         bool Connect( SlotBase* slot_base )
         {
            return BaseConnect( slot_base );
         }

		 bool Connect( void( *function_pointer )( T ) )
		 {
			 bool ret = false;

			 SANYAN_DEBUG_INFO("HASHES: " << typeid(T).hash_code() << " :: " << SignalType() << std::endl);
			 if( typeid( T ).hash_code() == SignalType() )
			 {
				 //we create a nameless functional slot here
				 //TODO: also create a connect that allows to pass
				 //in a function pointer and a name for the slot
				 //sanyan_slots_.push_back(new FunctinalSlot< T >( "", function_pointer ) );
             ret = BaseConnect( new FunctinalSlot< T >( "", function_pointer ) );
			 }
			 else
			 {
				 //TODO: Throw an error about type mismatch
				 ret = false;
			 }
			 return ret;
		 }

		 bool Disconnect( void( *function_pointer  )( T ) )
		 {
			 bool ret = false;
          ret = BaseDisconnect( ( void( *)( void* ) )function_pointer ); 
          return ret;
		 }

         void operator()( T arguments )
         {

            BaseEmit( ( void* )&arguments );
         }

      private:
         Signal(){};
        
   };

}

#endif