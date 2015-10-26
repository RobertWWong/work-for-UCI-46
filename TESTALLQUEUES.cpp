#include <iostream>
#include <sstream>
#include "ics46goody.hpp"
#include "array_stack.hpp"   // must leave in for constructor
#include "gtest/gtest.h"
#include "array_stack.hpp"   // must leave in for constructor
#include "linked_queue.hpp"




typedef ics::LinkedQueue<std::string> QueueType;
typedef ics::LinkedQueue<int>         QueueType2;

int main ()
{


	QueueType a ;
	QueueType b ;
	a.enqueue("why");
	a.enqueue("this");
	b.enqueue("project");
	std::cout<<"what"<<" what the heck\n";

	for (auto w = a)
		std::cout<< w <<"\n\n";


	return 0;

}
