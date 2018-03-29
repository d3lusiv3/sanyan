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
#else
#define SANYAN_DEBUG_INFO( msg )
#endif

//#define SIGNAL( name ) sanyan::Signal<> name = { #name }

//#define MEMBERSIGNAL( name, parentClass ) sanyan::Signal<> name = { #name, this }
#define MEMBERSIGNALARGS( name, type, parentClass ) sanyan::Signal< type > name = { #name, this }

#define MEMBERSLOT( name, type, parentClass )  void name( type ); \
   sanyan::MemberSlot< type, parentClass > name##Slot = { #name, this, &parentClass::name }



namespace sanyan
{


   //Define types to be used
   using Type_ID = size_t;
   using Unique_ID_Type = unsigned long long;

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
      SlotBase* GetSlot( std::string slot_name );

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
      std::string SlotName() const;
      Type_ID SlotType() const;
      void ReceiveBase( const void* arguments );
      Unique_ID_Type UUID() const;

      bool operator==( const SlotBase& rhs );
      bool operator==( const SlotBase* rhs );
      bool operator==( void( *const function_pointer )( void* ) );

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
      static Unique_ID_Type GenerateSlotUUID()
      {
         static Unique_ID_Type slot_uuid_generator_ = 0;
         return ++slot_uuid_generator_;
      }
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
      InheritableSlot( std::string slot_name ) : SlotBase( slot_name, typeid( T ).hash_code(), ( void( *)( void* ) )nullptr ) {}

      virtual void Receive( const void* arguments ) override
      {
         T* arg_pointer = reinterpret_cast<T*>( const_cast<void*>( arguments ) );
         OnReceived( *arg_pointer );
      }

      virtual void OnReceived( T arguments ) = 0;

   private:
      InheritableSlot(){};

   };


   template < class T >
   class FunctinalSlot : public SlotBase
   {
   public:
      FunctinalSlot( std::string slot_name, void( *function_pointer )( T ) ) : SlotBase( slot_name, typeid( T ).hash_code(), ( void( *)( void* ) )function_pointer ), function_pointer_( function_pointer ) {}

      virtual void Receive( const void* arguments ) override
      {
         T* p = reinterpret_cast<T*>( const_cast<void*>( arguments ) );
         ( *function_pointer_ )( ( *p ) );
      }

      bool operator==( void( *rhs )( T ) )
      {
         bool ret = false;
         if( function_pointer_ == rhs )
         {
            ret = true;
         }
         return ret;
      }

   private:
      FunctinalSlot(){}
      void( *function_pointer_ )( T );
   };

   template < >
   class FunctinalSlot< void > : public SlotBase
   {
   public:
      //we use 0 as typeid to represent void
      FunctinalSlot( std::string slot_name, void( *function_pointer )( void ) ) : SlotBase( slot_name, 0, ( void( *)( void* ) )function_pointer ), function_pointer_( function_pointer ) {}

      virtual void Receive( const void* arguments ) override
      {
         ( *function_pointer_ )( );
      }

      bool operator==( void( *rhs )( void ) )
      {
         bool ret = false;
         if( function_pointer_ == rhs )
         {
            ret = true;
         }
         return ret;
      }

   private:
      void( *function_pointer_ )( void );
   };


   template < class T, class C >
   class MemberSlot : public SlotBase
   {
   public:
      MemberSlot( std::string slot_name, C* const callback_parent_object, void( C::*member_function_callback )( T ) ) : SlotBase( slot_name, typeid( T ).hash_code(), callback_parent_object ), callback_parent_object_( callback_parent_object ), member_function_callback_( member_function_callback ){};

      virtual void Receive( const void* arguments ) override
      {
         T* arg_pointer = reinterpret_cast<T*>( const_cast<void*>( arguments ) );
         OnReceived( *arg_pointer );
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

   template < class C >
   class MemberSlot< void, C > : public SlotBase
   {
   public:
      //we use 0 to represent void as typeid
      MemberSlot( std::string slot_name, C* const callback_parent_object, void( C::*member_function_callback )( ) ) : SlotBase( slot_name, 0, callback_parent_object ), callback_parent_object_( callback_parent_object ), member_function_callback_( member_function_callback ) {};

      virtual void Receive( const void* arguments ) override
      {


         OnReceived();
      }

      void OnReceived()
      {
         ( callback_parent_object_->*member_function_callback_ )( void );
      }

   private:
      MemberSlot() {}
      C* const callback_parent_object_;
      void( C::*member_function_callback_ )( void );
   };



   class SignalingClass
   {
   public:
      SignalingClass();
      void RegisterSignal( SignalBase* signal_base );
      SignalBase* GetSignal( std::string signal_name );
   private:
      std::unordered_map< std::string, SignalBase* > signals_;

   };

   //Base Class for a signal
   class SignalBase : public SlotToSignalPrivateInterface
   {
   public:
      SignalBase( std::string signal_name, Type_ID type_id );
      SignalBase( std::string signal_name, Type_ID type_id, SignalingClass* const signaling_parent );
      virtual ~SignalBase();

      bool Connect( SlotBase* slot_base )
      {
         bool ret = false;
         if( slot_base->SlotType() == SignalType() )
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

      bool Connect( SlotBase& slot_base )
      {
         bool ret = false;
         if( slot_base.SlotType() == SignalType() )
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

      bool Disconnect( void( *function_pointer )( void* ) )
      {
         bool ret = false;
         for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin();
            ssIT != sanyan_slots_.end();
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

      bool Disconnect( SlotBase* slot_base )
      {
         bool ret = false;
         for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin();
            ssIT != sanyan_slots_.end();
            ++ssIT )
         {
            if( *( *ssIT ) == slot_base )
            {
               slot_base->UnregisterSignalBase( this );
               sanyan_slots_.erase( ssIT );
               ret = true;
               break;
            }
         }

         return ret;
      }

      bool Disconnect( SlotBase& slot_base )
      {
         bool ret = false;
         for( std::vector < SlotBase* >::iterator ssIT = sanyan_slots_.begin();
            ssIT != sanyan_slots_.end();
            ++ssIT )
         {
            if( *( *ssIT ) == ( &slot_base ) )
            {
               slot_base.UnregisterSignalBase( this );
               sanyan_slots_.erase( ssIT );
               ret = true;
               break;
            }
         }

         return ret;
      }


      Type_ID SignalType() const;
      std::string SignalName() const;
      SignalingClass* SignalingParent();


   protected:
      inline
         void BaseEmit( void* arguments )
      {

         for( int s = 0; s < sanyan_slots_.size(); ++s )
         {
            sanyan_slots_[ s ]->ReceiveBase( arguments );
         }


      }



   private:
      SignalBase();

      virtual void RemoveSlotOnDestruction( SlotBase* slot_base ) override
      {
         Disconnect( slot_base );
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

      Signal( std::string signal_name, SignalingClass* const signaling_parent )
         : SignalBase( signal_name, typeid( T ).hash_code(), signaling_parent )
      {
      }

      //one shot convenience signal to SlotBase
      Signal( const SlotBase& receiving_slot, T value )
         : SignalBase( "", typeid( T ).hash_code() )
      {
         receiving_slot.Receive( value );
      }

      Signal( void( *function_pointer )( T ), T value )
         : SignalBase( "", typeid( T ).hash_code() )
      {
         *function_pointer( value );
      }

      using SignalBase::Connect;

      bool Connect( void( *function_pointer )( T ) )
      {
         bool ret = false;

         if( typeid( T ).hash_code() == SignalType() )
         {
            //we create a nameless functional slot here
            //TODO: also create a connect that allows to pass
            //in a function pointfer and a name for the slot
            //TODO: we need to delete this new or we have a memory leak
            ret = Connect( new FunctinalSlot< T >( "", function_pointer ) );
         }
         else
         {
            //TODO: Throw an error about type mismatch
            ret = false;
         }
         return ret;
      }

      using SignalBase::Disconnect;

      bool Disconnect( void( *function_pointer )( T ) )
      {
         bool ret = false;
         ret = Disconnect( ( void( * )( void* ) )function_pointer );
         return ret;
      }

      inline void Emit( T& arguments )
      {
         BaseEmit( (void*)&arguments );
      }

      inline void Emit( const T& arguments )
      {
         BaseEmit( (void*)&arguments );
      }

      inline void Emit( T&& arguments )
      {
         BaseEmit( (void*)&arguments );
      }

      inline void Emit( const T&& arguments )
      {
         BaseEmit( (void*)&arguments );
      }

      inline void operator()( T& arguments )
      {

         BaseEmit( (void*)&arguments );
      }

      inline void operator()( const T& arguments )
      {

         BaseEmit( (void*)&arguments );
      }

      inline void operator()( T&& arguments )
      {

         BaseEmit( (void*)&arguments );
      }

      inline void operator()( const T&& arguments )
      {

         BaseEmit( (void*)&arguments );
      }

   private:
      Signal(){};

   };

   template < >
   class Signal< void > : public SignalBase
   {

   public:

      Signal( std::string signal_name )
         : SignalBase( signal_name, 0 )
      {
      }

      Signal( std::string signal_name, SignalingClass* const signaling_parent )
         : SignalBase( signal_name, 0, signaling_parent )
      {
      }

      //one shot convenience signal to SlotBase
      Signal( SlotBase& receiving_slot )
         : SignalBase( "", 0 )
      {
         char buff;
         receiving_slot.Receive( (void*)&buff );
      }

      Signal( void( *function_pointer )( void ) )
         : SignalBase( "", 0 )
      {

         ( *function_pointer )( );
      }

      using SignalBase::Connect;

      bool Connect( void( *function_pointer )( void ) )
      {
         bool ret = false;

         SANYAN_DEBUG_INFO( "HASHES: " << typeid( T ).hash_code() << " :: " << SignalType() << std::endl );
         if( ( (Type_ID)0 ) == SignalType() )
         {
            //we create a nameless functional slot here
            //TODO: also create a connect that allows to pass
            //in a function pointer and a name for the slot
            //TODO: we need to delete this new or we have a memory leak
            ret = Connect( new FunctinalSlot<void>( "", function_pointer ) );
         }
         else
         {
            //TODO: Throw an error about type mismatch
            ret = false;
         }
         return ret;
      }

      using SignalBase::Disconnect;

      bool Disconnect( void( *function_pointer )( void ) )
      {
         bool ret = false;
         ret = Disconnect( ( void( *)( void* ) )function_pointer );
         return ret;
      }


      void operator()()
      {
         char noargs;
         BaseEmit( (void*)&noargs );
      }

   private:
      //Signal( ) {};

   };

   inline 
   void 
   SlotBase::ReceiveBase( const void* arguments )
   {
      Receive( arguments );
   }

   inline
   SlottedClass::SlottedClass()
   {

   }

   inline
   void
   SlottedClass::RegisterSlot( SlotBase* slot_base )
   {
      slots_[ slot_base->SlotName() ] = slot_base;
   }

   inline
   SlotBase*
   SlottedClass::GetSlot( std::string name )
   {
      SlotBase* ret = nullptr;
      ret = slots_[ name ];
      return ret;
   }

   inline
   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, void( *base_function_pointer_ )( void* ) )
      : slot_name_( slot_name )
      , type_ID_( type_ID )
      , base_function_pointer_( base_function_pointer_ )
      , uuid_( GenerateSlotUUID() )
      , slotted_parent_( nullptr )
   {

   }

   inline
   SlotBase::SlotBase( std::string slot_name, Type_ID type_ID, SlottedClass* const slotted_parent )
      : slot_name_( slot_name )
      , type_ID_( type_ID )
      , slotted_parent_( slotted_parent )
      , uuid_( GenerateSlotUUID() )
   {
      slotted_parent->RegisterSlot( this );
   }

   inline
   SlotBase::~SlotBase()
   {

      std::vector< SignalBase* >::iterator csIT = connectedSignals_.begin();
      while( csIT != connectedSignals_.end() )
      {
         SignalBase* signal = ( *csIT );
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

   inline
   std::string
   SlotBase::SlotName() const
   {
      return slot_name_;
   }

   inline
   Type_ID
   SlotBase::SlotType() const
   {
      return type_ID_;
   }

   inline
   SlottedClass* const
   SlotBase::SlottedParent()
   {
      return slotted_parent_;
   }



   inline
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

   inline
   bool
   SlotBase::operator==( const SlotBase* rhs )
   {
      bool ret = false;
      if( uuid_ == rhs->UUID() )
      {
         ret = true;
      }
      return ret;
   }

   inline
   bool
   SlotBase::operator==( void( *const function_pointer )( void* ) )
   {
      bool ret = false;
      if( base_function_pointer_ == function_pointer )
      {
         ret = true;
      }
      return ret;
   }

   inline
   Unique_ID_Type
   SlotBase::UUID() const
   {
      return uuid_;
   }

   //private blocked default constructor
   inline
   SlotBase::SlotBase()
      : slotted_parent_( nullptr )
   {

   }

   inline
   SignalingClass::SignalingClass()
   {

   }

   inline
   void
   SignalingClass::RegisterSignal( SignalBase* signal_base )
   {
      signals_[ signal_base->SignalName() ] = signal_base;
   }

   inline
   SignalBase*
      SignalingClass::GetSignal( std::string signal_name )
   {
      SignalBase* ret = nullptr;
      ret = signals_[ signal_name ];
      return ret;
   }

   inline
   SignalBase::SignalBase( std::string signal_name, Type_ID type_id )
      : signal_name_( signal_name )
      , type_ID_( type_id )
      , signaling_parent_( nullptr )
   {
   }

   inline
   SignalBase::SignalBase( std::string signal_name, Type_ID type_id, SignalingClass* const signaling_parent )
      : signal_name_( signal_name )
      , type_ID_( type_id )
      , signaling_parent_( signaling_parent )
   {
      signaling_parent_->RegisterSignal( this );
   }

   inline
   SignalBase::~SignalBase()
   {
      for( int s = 0; s < sanyan_slots_.size(); ++s )
      {
         sanyan_slots_[ s ]->UnregisterSignalBase( this );
      }
   }

   inline
   std::string
   SignalBase::SignalName() const
   {
      return signal_name_;
   }

   inline
   SignalingClass*
   SignalBase::SignalingParent()
   {
      return signaling_parent_;
   }

   inline
   SignalBase::SignalBase()
      : signal_name_( "" )
      , type_ID_( -1 )
      , signaling_parent_( nullptr )
   {
   }

   inline
   Type_ID
   SignalBase::SignalType() const
   {
      return type_ID_;
   }

   inline
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
            ret = signal->Connect( slot );
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

   inline
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
            ret = signal->Disconnect( slot );
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

#endif