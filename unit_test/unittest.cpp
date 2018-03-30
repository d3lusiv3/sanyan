#include <iostream>
#include <string>
#include <chrono>
#include "Sanyan.hpp"

#define TESTPASSED std::cout << "PASSED" << std::endl
#define TESTFAILED std::cout << "FAILED" << std::endl; int failPause = 0; std::cin >> failPause
#define TESTINFO( info ) std::cout << "Testing " << info << "...."



//UNIT TEST FOR DOUBLE VALUE 
const double doubleTestValue = 243.537;

void DoubleSlotD( double d )
{
   if( d == doubleTestValue )
   {
      TESTPASSED;
   } else
   {
      TESTFAILED;
   }
}
double ddd;
void DoubleSlotEmpty( double d )
{
   //just to make sure compiler doesnt optimize it out, do something
   d += 1.1;
   ddd = d;
}

class DoubleInheritedSlot : public sanyan::InheritableSlot < double >
{
public:
   DoubleInheritedSlot() : sanyan::InheritableSlot< double >( "InheritedDoubleSlot" ){}
   virtual void OnReceived( double d ) override
   {
      if( d == doubleTestValue )
      {
         TESTPASSED;
      } else
      {
         TESTFAILED;
      }
   }
};

class TesterBase
{
public:
   void DoAll( double d )
   {
      for( int i = 0; i < all.size(); ++i )
      {
         all[ i ]->EmptyDouble( d );
      }
   }
   virtual void EmptyDouble( double d ) = 0;
   std::vector< TesterBase* > all;
};

class MemberSlotTester : public sanyan::SignalingClass, public sanyan::SlottedClass, public TesterBase
{
public:
   MEMBERSLOT( DoubleReceive, double, MemberSlotTester );
   MEMBERSIGNAL( DoubleUpdate, double, MemberSlotTester );
   MEMBERSIGNAL( DoublePUpdate, double*, MemberSlotTester );
   MEMBERSLOT( DoublePReceive, double*, MemberSlotTester );

   MemberSlotTester()
   {
      all.push_back( this );
   }
   virtual void EmptyDouble( double d ) override
   {
      d += 1.1;
   }




};

void
MemberSlotTester::DoubleReceive( double d )
{
   if( d == doubleTestValue )
   {
      TESTPASSED;
   } else
   {
      TESTFAILED;
   }
}

void SimpleSlotFunction()
{
   TESTPASSED;
}

void SimpleSlotFunction2()
{
   TESTFAILED;
}

//SIGNAL( SimpleSignal );
//sanyan::Signal<> test( "test");

void
MemberSlotTester::DoublePReceive( double* v )
{
   *v = 69.69;
}

void foovoid( void* args )
{

}



int main()
{



   bool connected = false;
   {
      TESTINFO( "Connecting void signal to void function pointer" );
      SIGNAL( voidSignal, void );
      connected = voidSignal.Connect( &SimpleSlotFunction );
      if( connected ) { TESTPASSED; } else { TESTFAILED; }
      //TESTINFO( "Connecting void signal to non-void function pointer" );
      //uncommment this to make sure you can not compile this 
      //connected = voidSignal.Connect( &DoubleSlotEmpty );
      //uncomment this to make sure you can not compile
      //sanyan::FunctinalSlot<void> voidSlot( "EmptySlot", &DoubleSlotEmpty );
      TESTINFO( "disconnecting previously NOT connected void function from void signal" );
      connected = voidSignal.Disconnect( &SimpleSlotFunction2 );
      if( !connected ) { TESTPASSED; } else { TESTFAILED; }
      TESTINFO( "disconnecting previously connected void function from void signal" );
      connected = voidSignal.Disconnect( &SimpleSlotFunction );
      if( connected ) { TESTPASSED; } else { TESTFAILED; }
      TESTINFO( "Connecting, disconnectiong and emitting void signal to void function pointer" );
      connected = true;
      connected = voidSignal.Connect( &SimpleSlotFunction2 );
      if( connected )
      {
         voidSignal.Disconnect( &SimpleSlotFunction2 );
         voidSignal();
         //if fail was not printed then we passed
         TESTPASSED;
      } else
      {
         //we didnt connect, failed for some other reason
         TESTFAILED;
      }

      sanyan::FunctinalSlot< void > voidFslot( "void slot", SimpleSlotFunction );
      TESTINFO( "Void functional slot connecting to void signal" );
      connected = false;
      connected = voidSignal.Connect( voidFslot );
      if( connected ) { TESTPASSED; } else { TESTFAILED; }
      TESTINFO( "Void Signal to void functional slot" );
      voidSignal();
      TESTINFO( "Disconnecting void functional slot from void signal" );
      connected = voidSignal.Disconnect( &voidFslot );
      if( connected ) { TESTPASSED; } else { TESTFAILED; }
      TESTINFO( "Signaling void signal to disconnected functional void slot" );
      voidSignal();
      //if got here we ok
      TESTPASSED;
      sanyan::FunctinalSlot< double >  doubleFSlot( "double slot", &DoubleSlotD );
      TESTINFO( "Connecting void signal to double functional slot" );
      connected = voidSignal.Connect( doubleFSlot );
      if( !connected ) { TESTPASSED; } else { TESTFAILED; }




   }

   MemberSlotTester* mstP = new MemberSlotTester();
   sanyan::SlottedClass* scP = mstP;
   sanyan::SignalingClass* sigP = mstP;

   sanyan::Signal< double > DoubleSig( "DoubleSig" );
   TESTINFO( "Connecting Functional Double Slot -> Double Signal" );
   connected = DoubleSig.Connect( DoubleSlotD );
   if( connected ){ TESTPASSED; } else{ TESTFAILED; }
   TESTINFO( "Double Signal RVALUE -> Functional Slot" );
   DoubleSig( 243.537 );
   TESTINFO( "Double Signal LVALUE -> Functional Slot" );
   DoubleSig( doubleTestValue );
   TESTINFO( "Disconnect double functional slot that WASNT previously hooked up" );
   double disconnected = true;
   disconnected = DoubleSig.Disconnect( DoubleSlotEmpty );
   if( !disconnected ){ TESTPASSED; } else{ TESTFAILED; }
   TESTINFO( "Disconnect double functional slot that WAS previously hooked up" );
   disconnected = false;
   disconnected = DoubleSig.Disconnect( DoubleSlotD );
   if( disconnected ){ TESTPASSED; } else{ TESTFAILED; }
   TESTINFO( "Double Disconnect double functional slot that WAS previously hooked up" );
   disconnected = false;
   disconnected = DoubleSig.Disconnect( DoubleSlotD );
   if( !disconnected ){ TESTPASSED; } else{ TESTFAILED; }

   {
      DoubleInheritedSlot dis;
      TESTINFO( "Connecting Double InheritetdSlot class to double signal" );
      connected = false;
      connected = DoubleSig.Connect( dis );
      if( connected ){ TESTPASSED; } else{ TESTFAILED; }
      TESTINFO( "Double Signal to Double Inherited Slot" );
      DoubleSig( doubleTestValue );
   }
   TESTINFO( "Double Signal to Double Inherited Slot That was destructed" );
   DoubleSig( doubleTestValue );
   //if we dont segfault here we passed
   TESTPASSED;

   {
      MemberSlotTester mt;
      TESTINFO( "Double signal connecting to double member slot" );
      connected = false;
      connected = DoubleSig.Connect( mt.DoubleReceiveSlot );
      if( connected ){ TESTPASSED; } else{ TESTFAILED; }
      TESTINFO( "Double signal emission to double member slot" );
      DoubleSig( doubleTestValue );
   }
   TESTINFO( "Double signal emission to deconstructed member slot" );
   DoubleSig( doubleTestValue );
   //again if no segfault we passed
   TESTPASSED;
   MemberSlotTester* mtP = new MemberSlotTester();
   TESTINFO( "Pointer object Double member signal connection to member slot " );
   connected = false;
   connected = mtP->DoubleUpdate.Connect( mtP->DoubleReceiveSlot );
   if( connected ){ TESTPASSED; } else{ TESTFAILED; }
   TESTINFO( "Pointer object Double member emission to member slot" );
   mtP->DoubleUpdate( doubleTestValue );
   TESTINFO( "Deleting pointer object with signals and slots" );
   delete mtP;
   //no segfaults we passed
   TESTPASSED;
   {
      MemberSlotTester mstR;
      MemberSlotTester* mstP = new MemberSlotTester();
      TESTINFO( "Connecting Stack signal to heap slot" );
      connected = false;
      connected = mstR.DoublePUpdate.Connect( mstP->DoublePReceiveSlot );
      if( connected ){ TESTPASSED; } else{ TESTFAILED; }
      double* dP = new double();
      *dP = 2342.446589374;
      TESTINFO( "Stack Signal to Heap Slot double pointer change value" );
      mstR.DoublePUpdate( dP );
      if( *dP == 69.69 ){ TESTPASSED; } else{ TESTFAILED; }
      delete mstP;
      TESTINFO( "Deleting previously connected heap slot and emitting stack signal" );
      *dP = 101.101;
      mstR.DoublePUpdate( dP );
      if( *dP == 101.101 ){ TESTPASSED; } else{ TESTFAILED; }
      delete dP;
   }
   {
      MemberSlotTester mstR;
      MemberSlotTester* mstP = new MemberSlotTester();
      connected = false;
      TESTINFO( "Connecting stack slot by string to heap signal by string" );
      connected = sanyan::OBJECT_CONNECT( &mstR, "DoubleUpdate", mstP, "DoubleReceive" );
      if( connected ){ TESTPASSED; } else{ TESTFAILED; }
      TESTINFO( "Signaling stack signal to heap slot connected by string lookup" );
      mstR.DoubleUpdate( doubleTestValue );
      TESTINFO( "Disconnecting stack signal to heap slot by string lookup" );
      disconnected = false;
      disconnected = sanyan::OBJECT_DISCONNECT( &mstR, "DoubleUpdate", mstP, "DoubleReceive" );
      if( disconnected ){ TESTPASSED; } else{ TESTFAILED; }
      TESTINFO( "Signaling stack signal to disconnected heap slot" );
      mstR.DoubleUpdate( doubleTestValue );
      //we got here, if multiple passes appear then its actually a fail
      TESTPASSED;
      TESTINFO( "Reconnecting stack signal to previously disconnected heap slot" );
      connected = false;
      connected = sanyan::OBJECT_CONNECT( &mstR, "DoubleUpdate", mstP, "DoubleReceive" );
      if( connected ){ TESTPASSED; } else{ TESTFAILED; }
      TESTINFO( "Resignaling stack signal to previously disconnected but reconnected heap slot" );
      mstR.DoubleUpdate( doubleTestValue );
      delete mstP;
      TESTINFO( "Signaling stack signal to deleted heap slot connected by string lookup" );
      mstR.DoubleUpdate( doubleTestValue );
      //didnt segfault or if we get two returns for pass/fail we failed
      TESTPASSED;
   }

   {
      MemberSlotTester mstR1;
      MemberSlotTester mstR2;
      TESTINFO( "Connecting non existint signal to existing slot" );
      connected = true;
      connected = sanyan::OBJECT_CONNECT( &mstR1, "DoesntExist", &mstR2, "DoubleReceive" );
      if( !connected ) { TESTPASSED; } else{ TESTFAILED; }


   }

   {
      //do some timing benchmarks compared to other approaches
      //first call a double passed in by copy
      unsigned long long iterationsToRun = 30000000;


      double localDValue = 101.33;
      MemberSlotTester mst;
      TesterBase* tbP = &mst;

      auto t1 = std::chrono::high_resolution_clock::now();
      for( unsigned long long i = 0; i < iterationsToRun; ++i )
      {
         tbP->DoAll( localDValue );
      }
      auto t2 = std::chrono::high_resolution_clock::now();
      double diff1 = (double)std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1 ).count();
      std::cout << "Benchmarking double pure virtual call by copy: "
         << diff1
         << " nanoseconds" << "which took: " << ( diff1 ) << " seconds" << std::endl;


      sanyan::FunctinalSlot< double > fdslot( "bench_d_slot", &DoubleSlotEmpty );
      sanyan::Signal< double > fdsignal( "bench_d_signal" );
      fdsignal.Connect( fdslot );

      t1 = std::chrono::high_resolution_clock::now();
      for( unsigned long long i = 0; i < iterationsToRun; ++i )
      {
         fdsignal( localDValue );
      }
      t2 = std::chrono::high_resolution_clock::now();
      double diff2 = (double)std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1 ).count();
      std::cout << "Benchmarking double functional signal to functional slot: "
         << diff2
         << " nanoseconds" << "which is: " << ( diff2 / diff1 ) << " times slower than baseline" << std::endl;




   }






   int stop;
   std::cin >> stop;
   return 0;

}