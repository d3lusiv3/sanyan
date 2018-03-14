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

int main()
{
	
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
	double doubleLvalue = 243.537;
	DoubleSig(doubleLvalue);
	TESTINFO("Disconnect Functional Double Slot from Double Signal");
	double disconnected = DoubleSig.Disconnect(DoubleSlotD);
	if (disconnected)
	{
		TESTPASSED;
	}
	else
	{
		TESTFAILED;
	}
	TESTINFO("Double Sig RVALUE -> Double InheritedSlot");
	DoubleInheritedSlot diss;
	


	int stop;
	std::cin >> stop;
	return 0;

}