#ifndef LINKED_QUEUE_HPP_
#define LINKED_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedQueue {
  public:
    //Destructor/Constructors
    ~LinkedQueue();

    LinkedQueue          ();
    LinkedQueue          (const LinkedQueue<T>& to_copy);
    explicit LinkedQueue (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedQueue (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    LinkedQueue<T>& operator = (const LinkedQueue<T>& rhs);
    bool operator == (const LinkedQueue<T>& rhs) const;
    bool operator != (const LinkedQueue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T2>& q);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedQueue<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedQueue<T>::Iterator& operator ++ ();
        LinkedQueue<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedQueue<T>::Iterator& rhs) const;
        bool operator != (const LinkedQueue<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedQueue<T>::begin () const;
        friend Iterator LinkedQueue<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev = nullptr;  //if nullptr, current at front of list
        LN*             current;         //current == prev->next (if prev != nullptr)
        LinkedQueue<T>* ref_queue;		//NO. THIS IS A ITERATOR
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedQueue<T>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next = nullptr;
    };


    LN* front     =  nullptr;
    LN* rear      =  nullptr;
    int used      =  0;            //Cache for number of values in linked list
    int mod_count =  0;            //For sensing of a concurrent modification

    //Helper methods
    void delete_list(LN*& front);  //Deallocate all LNs, and set front's argument to nullptr;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedQueue class and related definitions

//Destructor/Constructors

template<class T>
LinkedQueue<T>::~LinkedQueue() {
	//how to delete all lonked nodes
	this->delete_list(front);
}


template<class T>
LinkedQueue<T>::LinkedQueue() {

}


template<class T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue<T>& to_copy) {

}


template<class T>
LinkedQueue<T>::LinkedQueue(const std::initializer_list<T>& il) {
}

// ics::LinkedQueue<int> q({1,2,3});

template<class T>
template<class Iterable>
LinkedQueue<T>::LinkedQueue(const Iterable& i)
 {
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedQueue<T>::empty() const {
	return front == nullptr;
}


template<class T>
int LinkedQueue<T>::size() const {
	return used;
}


template<class T>
T& LinkedQueue<T>::peek () const {
	if (this->empty())
		throw EmptyError("LinkedQueue::peek");
	return front->value; // dont use a variable to store the reference value
	//it will go out of the stack when you return it. THIS IS A LOCAL OBJECT.
	//DONT FORGET ABOUT..
}


template<class T>
std::string LinkedQueue<T>::str() const {
	std::ostringstream answer ;
	answer <<"queue[";

	//HEY BE CAREFUL OVER HERE, IF YOU GET AN IN READY ONLY ERROR, IT MEANS YOU
	//SHOULDN'T HAVE TRIED TO MODIFY A CONST VALUE
	//previous, i did front = front->next, con't do that because it was a const value.
	if (!this->empty())
		{	answer << front->value;
			for (auto temp = front->next; temp != nullptr; temp = temp->next)
			{
				answer << ","<<temp->value;
			}
		}
	return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T>
int LinkedQueue<T>::enqueue(const T& element) {
	LN* list_to_add = new LN(element,nullptr);	//using new creates the variable in th GLOBAL SCOPE. THAT'S WHAT NEW IS FOR
 	if (front == nullptr && rear == nullptr)
 		rear = front =  list_to_add; // got em
	else
		{
		rear->next = list_to_add ;//this will make sure our value doesn't get destroyed.
		rear = rear->next;	//MAKE SURE YOU CHANGE REAR TO REAR->NEXT. PREASE.
		}
	used++;
	mod_count++;
	return 1;

}


template<class T>
T LinkedQueue<T>::dequeue() {
	if (this->empty())
		throw EmptyError("LinkedQueue::dequeue");

	T answer = front->value;
	front = front->next;
	mod_count++;
	used--;
	return answer;
}


template<class T>
void LinkedQueue<T>::clear() {	//This is a queue in linked list format. Not an array queue.
	while (front != nullptr)
	{
		auto delete_Front = front;
		front= front->next;
		delete delete_Front;
	}
	front = rear = nullptr;
	used =0;
	mod_count++;
}

/*HEY
YOU WANT TO USE THIS FUNCTION?
CREATE THE ITERATOR,
++ OPERAND,
AND ANYTHING RELATED TO AUTO-ING

for (auto k = i.being(); k != i.end(); k++)
{
auto x = *i;
do something with x

*/
template<class T>
template<class Iterable>
int LinkedQueue<T>::enqueue_all(const Iterable& i) {
	int count =0;
	for (const T& v: i)
		count += enqueue(v);
	return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedQueue<T>& LinkedQueue<T>::operator = (const LinkedQueue<T>& rhs) {
	if (this == &rhs)
		return *this;

	front = nullptr;
	rear = nullptr;

	//NEED HELP HERE.

}

////NEED TO WORK ON THIS ONE.
//	front = nullptr;
//	rear =nullptr;


template<class T>
bool LinkedQueue<T>::operator == (const LinkedQueue<T>& rhs) const {

}


template<class T>
bool LinkedQueue<T>::operator != (const LinkedQueue<T>& rhs) const {
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>& q) {
	outs<<"queue[";

	if (!q.empty())// checks if empty
	{
		outs<<q.front->value;//put into stream your first value
		for (auto temp = q.front->next; temp != nullptr; temp = temp->next)
			outs<<","<< temp->value;
	}//above loop will go through each node and output its value. will stop once nullptr is reached.
	outs << "]:rear";
	return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedQueue<T>::begin () const -> LinkedQueue<T>::Iterator {
	return Iterator(const_cast<LinkedQueue<T>*>(this), front);
}

template<class T>
auto LinkedQueue<T>::end () const -> LinkedQueue<T>::Iterator {
	return Iterator(const_cast<LinkedQueue<T>*>(this), rear);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
void LinkedQueue<T>::delete_list(LN*& front) {
	while (front != nullptr)
	{
		auto to_delete = front;
		front = front->next;
		delete to_delete;
	}
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedQueue<T>::Iterator::Iterator(LinkedQueue<T>* iterate_over, LN* initial)
: current (initial), ref_queue(iterate_over), expected_mod_count(ref_queue->mod_count)
{	//That's it.
}


template<class T>
LinkedQueue<T>::Iterator::~Iterator()
{}


template<class T>
T LinkedQueue<T>::Iterator::erase() {

	if (expected_mod_count != ref_queue->mod_count)
		throw ConcurrentModificationError("LinkedQueue::Iterator::erase");
	if (!can_erase)
	    throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor already erased");
//	if (!ref_queue->is_in(current))
//	    throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor beyond data structure");

	can_erase = false;

}


template<class T>
std::string LinkedQueue<T>::Iterator::str() const {
	std::ostringstream answer;
	answer << ref_queue->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
	return answer.str();
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ () -> LinkedQueue<T>::Iterator& {

	if (expected_mod_count != ref_queue->mod_count)
		throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++");

	if (current == ref_queue->rear)
		return *this;

	if (can_erase)
		current = prev->next;
	else
		can_erase = true;

	return *this;
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ (int) -> LinkedQueue<T>::Iterator {

	if (expected_mod_count != ref_queue->mod_count)
		throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++");

	if (current == ref_queue->rear)
		return *this;

	Iterator to_return (*this);
	if (can_erase)
		current = prev->next;
	else
		can_erase = true;

	return to_return;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator == (const LinkedQueue<T>::Iterator& rhs) const {
	const Iterator* rhsASI =dynamic_cast<const Iterator*> (&rhs);

	if (rhsASI = nullptr)
		throw IteratorTypeError("LinkedQueue::Iterator::operator ==");
	if (expected_mod_count != ref_queue->mod_count)
		throw ConcurrentModificationError ("Iterator::operator ==");
	if (ref_queue != rhsASI->ref_queue)
		throw ComparingDifferentIteratorsError ("Iterator::operator ==");

	return current == rhsASI->current;



}


template<class T>
bool LinkedQueue<T>::Iterator::operator != (const LinkedQueue<T>::Iterator& rhs) const {
	 const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
	  if (rhsASI == nullptr)
	    throw IteratorTypeError("ArrayQueue::Iterator::operator !=");
	  if (expected_mod_count != ref_queue->mod_count)
	    throw ConcurrentModificationError("ArrayQueue::Iterator::operator !=");
	  if (ref_queue != rhsASI->ref_queue)
	    throw ComparingDifferentIteratorsError("ArrayQueue::Iterator::operator !=");

	  return current != rhsASI->current;
}


template<class T>
T& LinkedQueue<T>::Iterator::operator *() const {
}


template<class T>
T* LinkedQueue<T>::Iterator::operator ->() const {
}


}

#endif /* LINKED_QUEUE_HPP_ */
