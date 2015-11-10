#ifndef HEAP_PRIORITY_QUEUE_HPP_
#define HEAP_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include <utility>              //For std::swap function
#include "array_stack.hpp"      //See operator <<


namespace ics {


//Instantiate the templated class supplying tgt(a,b): true, iff a has higher priority than b.
//If tgt is defaulted to nullptr in the template, then a constructor must supply cgt.
//If both tgt and cgt are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-nullptr value supplied by tgt/cgt is stored in the instance variable gt.
template<class T, bool (*tgt)(const T& a, const T& b) = nullptr> class HeapPriorityQueue {
  public:
    //Destructor/Constructors
    ~HeapPriorityQueue();

    HeapPriorityQueue          (bool (*cgt)(const T& a, const T& b) = nullptr);
    explicit HeapPriorityQueue (int initial_length, bool (*cgt)(const T& a, const T& b));
    HeapPriorityQueue          (const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b) = nullptr);
    explicit HeapPriorityQueue (const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b) = nullptr);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HeapPriorityQueue (const Iterable& i, bool (*cgt)(const T& a, const T& b) = nullptr);


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
    HeapPriorityQueue<T,tgt>& operator = (const HeapPriorityQueue<T,tgt>& rhs);
    bool operator == (const HeapPriorityQueue<T,tgt>& rhs) const;
    bool operator != (const HeapPriorityQueue<T,tgt>& rhs) const;

    template<class T2, bool (*gt2)(const T2& a, const T2& b)>
    friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T2,gt2>& pq);



    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of HeapPriorityQueue<T,tgt>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HeapPriorityQueue<T,tgt>::Iterator& operator ++ ();
        HeapPriorityQueue<T,tgt>::Iterator  operator ++ (int);
        bool operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        bool operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }

        friend Iterator HeapPriorityQueue<T,tgt>::begin () const;
        friend Iterator HeapPriorityQueue<T,tgt>::end   () const;

      private:
        //If can_erase is false, the value has been removed from "it" (++ does nothing)
        HeapPriorityQueue<T,tgt>  it;                 //copy of HPQ (from begin), to use as iterator via dequeue
        HeapPriorityQueue<T,tgt>* ref_pq;
        int                      expected_mod_count;
        bool                     can_erase = true;

        //Called in friends begin/end
        //These constructors have different initializers (see it(...) in first one)
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool from_begin);    // Called by begin
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over);                     // Called by end
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    bool (*gt) (const T& a, const T& b); // The gt used by enqueue (from template or constructor)
    T*  pq;                              // Smaller values in lower indexes (biggest is at used-1)
    int length    = 0;                   //Physical length of array: must be >= .size()
    int used      = 0;                   //Amount of array used:  invariant: 0 <= used <= length
    int mod_count = 0;                   //For sensing concurrent modification


    //Helper methods
    void ensure_length  (int new_length);
    int  left_child     (int i) const;         //Useful abstractions for heaps as arrays
    int  right_child    (int i) const;
    int  parent         (int i) const;
    bool is_root        (int i) const;
    bool in_heap        (int i) const;
    void percolate_up   (int i);
    void percolate_down (int i);
    void heapify        ();                   // Percolate down all value is array (from indexes used-1 to 0): O(N)
  };





////////////////////////////////////////////////////////////////////////////////
//
//HeapPriorityQueue class and related definitions

//Destructor/Constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::~HeapPriorityQueue() {
	delete [] pq;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(bool (*cgt)(const T& a, const T& b))
: gt(tgt != nullptr ? tgt: cgt)
{
	if (gt == nullptr)	//must supply gt function
	    throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
	if (tgt != nullptr &&  cgt != nullptr && tgt != cgt)	//if both comp function is nullptr or both or not equal to each other.
	    throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");

	pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(int initial_length,
		bool (*cgt)(const T& a, const T& b))
: gt(tgt != nullptr ? tgt: cgt), length(initial_length)
{
	if (gt == nullptr)	//must supply gt function
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
	if (tgt != nullptr &&  cgt != nullptr && tgt != cgt)	//if both comp function aren't nullptr and are not equal to each other.
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");
	if (length <0)
		length = 0;
	pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b))
: gt(tgt != nullptr ? tgt: cgt), length(to_copy.length), used (to_copy.used)
{
	if (gt == nullptr)	//must supply gt function
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
	if (tgt != nullptr &&  cgt != nullptr && tgt != cgt)	//if both comp function is nullptr or both or not equal to each other.
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");

	pq = new T[length];

	if (cgt == to_copy.gt)
	{
 		for (int i = 0; i <to_copy.used; i++)
			pq[i] = to_copy.pq[i];
	}
	else		//MUST CALL HEAPIFY INTO MAX HEAP TREE.
	{
		for (int i = 0; i <to_copy.used; i++)
			pq[i] = to_copy.pq[i];
		heapify();
	}
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const std::initializer_list<T>& il,
		bool (*cgt)(const T& a, const T& b))
: gt(tgt != nullptr ? tgt : cgt) {

	if (gt == nullptr)	//must supply gt function
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
	if (tgt != nullptr &&  cgt != nullptr && tgt != cgt)	//if both comp function is nullptr or both or not equal to each other.
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");

	for (const T& q_elem : il)
		enqueue(q_elem);
	heapify();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template<class Iterable>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const Iterable& i,
		bool (*cgt)(const T& a, const T& b))
: gt(tgt != nullptr ? tgt : cgt) {
	if (gt == nullptr)
		throw TemplateFunctionError("HeapPriorityQueue::iterable constructor: neither specified");
	if (tgt != nullptr &&  cgt != nullptr && tgt != cgt)	//if both comp function is nullptr or both or not equal to each other.
		throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");

	for (const T& v: i)
		enqueue(v);
	heapify();
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::empty() const {
	return used == 0;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::size() const {
	return used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::peek () const {
	if (empty())
		throw EmptyError("HeapPriorityQueue::peek()");
	return pq[0];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::str() const {
	std::ostringstream answer;
	answer << *this << "(length)=" <<length<< ",used="<< used << ",mod_count=" << mod_count<<")";
	return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::enqueue(const T& element) {
	this->ensure_length(used +1);	//only makes new array when we have too many values.
	pq[used++] = element;	//used already incremented

	percolate_up(used-1);	// work from bottom up, add to the end, and work way up to preserve
	//order of the tree
	++mod_count;
	return 1;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::dequeue() {
	if (this->empty())
		throw EmptyError("HeapPriorityQueue::dequeue");

	T topVal = pq[0];
	//Alright. Make top value equal to the last value of the tree (it will be at the bottom of the tree. Convienent.
	pq [0] = pq[--used];
	percolate_down(0); //Here's the brunt of the work, percolating it down now.

	return topVal;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::clear() {
	used = 0;
	++mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template <class Iterable>
int HeapPriorityQueue<T,tgt>::enqueue_all (const Iterable& i) {
	int count = 0;
	for ( const T v :i)
		count += enqueue(v);
	return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>& HeapPriorityQueue<T,tgt>::operator = (const HeapPriorityQueue<T,tgt>& rhs) {
	if (this == &rhs)
		return *this;
	this->ensure_length(rhs.used);
	gt = rhs.gt;
	used = rhs.used;

	for (int i = 0; i < used; ++i)
	{
		pq[i] = rhs.pq[i];
		std::cout << "pq[" << i << "] = " << pq[i] << ", rhs.pq[" << i << "] = " << rhs.pq[i] << std::endl;
	}
	++mod_count;
	return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator == (const HeapPriorityQueue<T,tgt>& rhs) const {
	if (this == &rhs)
		return true;
	if (used != rhs.size() || gt != rhs.gt)
		return false;

	HeapPriorityQueue<T,tgt> toCopy = *this;
	HeapPriorityQueue<T,tgt>::Iterator rhs_i = rhs.begin();

	for (int i = 0 ; i < used; ++i, ++rhs_i)
		if (toCopy.dequeue() != *rhs_i)
			return false;

	return true;


}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator != (const HeapPriorityQueue<T,tgt>& rhs) const {
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>& p) {
	outs <<"priority_queue[";

	T sort_list  [p.used];	//frustration. using built in sort function to give me how the function looks like. This is probably wrong
	for (int i = 0; i <p.used ; i++)
		sort_list [i] = p.pq[i];
	std::sort (sort_list , sort_list +p.used);


	if (!p.empty())
	{
		for (int i = p.used-1; i >=0; --i)
		{
			if (i == p.used-1)
				outs<<sort_list[i];	//normally would us p.pq[i] so rever back if not right.
			else
				outs<< "," << sort_list [i];
		}
	}
	outs<< "]:highest";
	return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::begin () const -> HeapPriorityQueue<T,tgt>::Iterator {
	  return Iterator(const_cast<HeapPriorityQueue<T,tgt>*>(this),true);

 }


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::end () const -> HeapPriorityQueue<T,tgt>::Iterator {
	  return Iterator(const_cast<HeapPriorityQueue<T,tgt>*>(this),false);

 }



////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::ensure_length(int new_length) {
	if (length >= new_length)
		return;	//we want to make sure that our current length is c
	T *old_pq = pq;//make copy of old pq
	length = std::max(new_length, 2*length);// new length will be max of either the
	//newlength or twice that of old length (in the case
	pq = new T[length];//create that new array
	for ( int i = 0; i <used ; ++i)
		pq[i] = old_pq[i];//copy over the values of old to new

	delete [] old_pq;
}

//this part was on his heap page, had to ctrl-f left child

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::left_child(int i) const
{
	return 2*i + 1;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::right_child(int i) const
{
	return 2*i + 2;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::parent(int i) const
{
	return (i-1)/2;	//this is the formula to find out who the parent is for a certain child
		//let's say we have 7 nodes, we round down.
		// ..f
		// .e
		// a			let's say we chose i=2, which is child c
		// ..d		//preorder duh [a, b,c,d,e,f]
		// .b		// (2-1)/2 =1 ;;;;; duh [1] =b
		// ..c
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::is_root(int i) const
{
	return i == 0;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::in_heap(int i) const
{
	return (i <used);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_up(int i) {
	//This is for enqueue, if the value is not a root
//	std::string words;
//	for (int i =0; i <used ; i++)
//				words += pq[i];

	for (; !is_root(i) && gt(pq[i], pq[parent(i)]) ; i = parent(i))
	{
		//make sure that it is not a root, and check which is greater
		// the parent value or the current value,
		//Then if the swap parent with child if child is greater or it moves it up
//		std::cout<<"PQ in progress: " << words<<
//				"\nHere is the Values to be removed : parent = " <<pq[parent(i)]<<" and child = " <<pq[i]<<
//				"\n\n";
		std::swap(pq[parent(i)], pq[i]);

	}

}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_down(int i) {
	for (int left = left_child(i) ; in_heap(left) ; left = left_child(left))
	{

		//let's say i = 0, we have left = 1 and right = 2
		int right = right_child(i);	//returns index value of child node of that parent

		//Dual check : check first to see if right child is in the heap. If not, use left child as comparison
		int max_val ;
		if (!in_heap(right) || gt( pq[left], pq [right]))
			max_val = left;
		else
			max_val = right;
		//if right child is in the heap, now compare the value between left and right child. Used value depends on comp func

		if (gt(pq[i], pq[max_val]))		//if parent is less/greater than its child, stop out of that loop
			break;
		std::swap ( pq[i], pq[max_val]);	//if comparison condition fails, then swap the values
		i = max_val; //here's your tracker. Set your parent index to that of which you swapped with to
		//ensure correct order tracking. Jasus

	}
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::heapify() {
for (int i = used-1; i >= 0; --i)
  percolate_down(i);
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool tgt_nullptr)
: it() , ref_pq(iterate_over)
{
	if (tgt_nullptr)
		it = *ref_pq;
	expected_mod_count = ref_pq->mod_count;
}



template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over)
: it (iterate_over)
{
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::~Iterator()
{}


template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::Iterator::erase() {
	if (expected_mod_count != ref_pq->mod_count)
		throw ConcurrentModificationError("HeapPriorityQueue::Iterator::erase");
	if (!can_erase)
		throw CannotEraseError("HeapPriorityQueue::Iterator::erase Iterator cursor already erased");
	if (it.empty())
		throw CannotEraseError("HeapPriorityQueue::Iterator::erase Iterator cursor beyond data structure");

	can_erase = false; //
	T top_val = it.peek();
	int index;
	for (int i = 0 ; i < ref_pq->used; i++)
	{
		if (it.peek() == ref_pq->pq[i])
		{
			index = i;
			break;
		}
	}

	ref_pq->pq[index] = ref_pq->pq[ref_pq->used-1];
	it.dequeue();
	ref_pq->used--;
	ref_pq->percolate_down(index);
	ref_pq->percolate_up(index);

	expected_mod_count = ref_pq->mod_count;
	return top_val;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::Iterator::str() const {
	std::ostringstream answer;
	answer << ref_pq->str() << "/current_value=" << it.peek() << "/expected_mod_count=" << expected_mod_count << "/can_erase=" << can_erase;  // ASDFASDF?
	return answer.str();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ () -> HeapPriorityQueue<T,tgt>::Iterator& {
	if (expected_mod_count != ref_pq->mod_count)
		throw ConcurrentModificationError("HeapPriorityQueue<T,tgt>::Iterator::operator ++");

	if (it.empty())
		return *this;
	if (!can_erase)
		can_erase = true;
	else
		it.dequeue();
	return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ (int) -> HeapPriorityQueue<T,tgt>::Iterator {
	if (expected_mod_count != ref_pq->mod_count)
		throw ConcurrentModificationError("HeapPriorityQueue<T,tgt>::Iterator::operator ++");
	if (it.empty())
		return *this;

	Iterator *to_return = new Iterator(*this);
	if (!can_erase)
		can_erase = true;
	else
		it.dequeue();

	return *to_return;


}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::Iterator::operator *() const {
	if (expected_mod_count != ref_pq->mod_count)
		throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator *");
	if (!can_erase || it.empty())
	{
		std::ostringstream where;
		where << it.peek() << " when size = " << ref_pq->size();
		throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator * Iterator illegal: "+where.str());
	}

	return it.peek();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T* HeapPriorityQueue<T,tgt>::Iterator::operator ->() const {
	if (expected_mod_count !=  ref_pq->mod_count)
			throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ->");
	if (!can_erase || it.empty())
	{
		std::ostringstream where;
		where << it.peek() << " when size = " << ref_pq->size();
		throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator -> Iterator illegal: "+where.str());
	}

	return &(it.peek());
}

}

#endif /* HEAP_PRIORITY_QUEUE_HPP_ */
