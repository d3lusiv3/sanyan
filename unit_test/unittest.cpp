#include <iostream>
#include "Sanyan.hpp"

#define TESTPASSED std::cout << "PASSED" << std::endl
#define TESTFAILED std::cout << "FAILED" << std::endl
#define TESTINFO( info ) std::cout << "Testing " << info << "...."



//UNIT TEST FOR DOUBLE VALUE 
const double doubleTestValue = 243.537;

void DoubleSlotD( double d )
{
	if (d == doubleTestValue)
	{
		TESTPASSED;
	}
	else
	{
		TESTFAILED;
	}
}

void DoubleSlotEmpty( double d )
{

}

class DoubleInheritedSlot : public sanyan::InheritableSlot< double >
{
public:
	DoubleInheritedSlot() : sanyan::InheritableSlot< double >("InheritedDoubleSlot"){}
	virtual void OnReceived(double d) override
	{
		if (d == doubleTestValue)
		{
			TESTPASSED;
		}
		else
		{
			TESTFAILED;
		}
	}
};

class MemberSlotTester : public sanyan::SignalingClass, public sanyan::SlottedClass
{
public:
	MEMBERSLOT(DoubleReceive, double, MemberSlotTester);
	MEMBERSIGNAL(DoubleUpdate, double, MemberSlotTester);
	
};

void
MemberSlotTester::DoubleReceive(double d)
{
	if (d == doubleTestValue)
	{
		TESTPASSED;
	}
	else
	{
		TESTFAILED;
	}
}

int main()
{
	
	MemberSlotTester* mstP = new MemberSlotTester();
	sanyan::SlottedClass* scP = mstP;
	sanyan::SignalingClass* sigP = mstP;
	std::cout << sizeof(sanyan::SlottedClass);

	sanyan::Signal< double > DoubleSig( "DoubleSig" );
	TESTINFO("Connecting Functional Double Slot -> Double Signal");
	bool connected = DoubleSig.Connect( DoubleSlotD );
	if (connected)
	{
		TESTPASSED;
	}
	else
	{
		TESTFAILED;
	}
	TESTINFO("Double Signal RVALUE -> Functional Slot");
	DoubleSig( 243.537 );
	TESTINFO("Double Signal LVALUE -> Functional Slot");
	DoubleSig(doubleTestValue);
   TESTINFO( "Disconnect double functional slot that WASNT previously hooked up");
   double disconnected = true;
   disconnected = DoubleSig.Disconnect( DoubleSlotEmpty );
	if (!disconnected)
	{
		TESTPASSED;
	}
	else
	{
		TESTFAILED;
	}
   TESTINFO( "Disconnect double functional slot that WAS previously hooked up" );
   disconnected = false;
   disconnected = DoubleSig.Disconnect( DoubleSlotD ); 
   if( disconnected )
   {
      TESTPASSED;
   }
   else
   {
      TESTFAILED;
   }
   TESTINFO( "Double Disconnect double functional slot that WAS previously hooked up" );
   disconnected = false;
   disconnected = DoubleSig.Disconnect( DoubleSlotD );
   if( !disconnected )
   {
      TESTPASSED;
   }
   else
   {
      TESTFAILED;
   }

   {
      DoubleInheritedSlot dis;
      TESTINFO( "Connecting Double InheritetdSlot class to double signal" );
      connected = false;
      connected = DoubleSig.Connect( dis );
      if( connected )
      {
         TESTPASSED;
      }
      else
      {
         TESTFAILED;
      }
      TESTINFO( "Double Signal to Double Inherited Slot");
	  DoubleSig(doubleTestValue);
   }
   TESTINFO( "Double Signal to Double Inherited Slot That was destructed" );
   DoubleSig(doubleTestValue);
   //if we dont segfault here we passed
   TESTPASSED;

   {
	   MemberSlotTester mt;
	   TESTINFO("Double signal connecting to double member slot");
	   connected = false;
	   connected = DoubleSig.Connect(mt.DoubleReceiveSlot);
	   if (connected)
	   {
		   TESTPASSED;
	   }
	   else
	   {
		   TESTFAILED;
	   }
	   TESTINFO("Double signal emission to double member slot");
	   DoubleSig(doubleTestValue);
   }
   TESTINFO("Double signal emission to deconstructed member slot");
   DoubleSig(doubleTestValue);
   //again if no segfault we passed
   TESTPASSED;
   MemberSlotTester* mtP = new MemberSlotTester();
   TESTINFO("Pointer object Double member signal connection to member slot ");
   connected = false;
   connected = mtP->DoubleUpdate.Connect(mtP->DoubleReceiveSlot);
   if (connected)
   {
	   TESTPASSED;
   }
   else
   {
	   TESTFAILED;
   }
   TESTINFO("Pointer object Double member emission to member slot");
   mtP->DoubleUpdate(doubleTestValue);
   TESTINFO("Deleting pointer object with signals and slots");
   delete mtP;
   //no segfaults we passed
   TESTPASSED;

   


	int stop;
	std::cin >> stop;
	return 0;

}