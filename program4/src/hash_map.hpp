//Robert Wong (547710)
//Kenneth Dy (419078)


#ifndef HASH_MAP_HPP_
#define HASH_MAP_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"


namespace ics {


//Instantiate the templated class supplying thash(a): produces a hash value for a.
//If thash is defaulted to nullptr in the template, then a constructor must supply chash.
//If both thash and chash are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-nullptr value supplied by thash/chash is stored in the instance variable hash.
template<class KEY,class T, int (*thash)(const KEY& a) = nullptr> class HashMap {
  public:
    typedef ics::pair<KEY,T>   Entry;

    //Destructor/Constructors
    ~HashMap ();

    HashMap          (double the_load_threshold = 1.0, int (*chash)(const KEY& a) = nullptr);
    explicit HashMap (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const KEY& k) = nullptr);
    HashMap          (const HashMap<KEY,T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = nullptr);
    explicit HashMap (const std::initializer_list<Entry>& il, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = nullptr);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashMap (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = nullptr);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool has_key    (const KEY& key) const;
    bool has_value  (const T& value) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    T    put   (const KEY& key, const T& value);
    T    erase (const KEY& key);
    void clear ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int put_all(const Iterable& i);


    //Operators

    T&       operator [] (const KEY&);
    const T& operator [] (const KEY&) const;
    HashMap<KEY,T,thash>& operator = (const HashMap<KEY,T,thash>& rhs);
    bool operator == (const HashMap<KEY,T,thash>& rhs) const;
    bool operator != (const HashMap<KEY,T,thash>& rhs) const;

    template<class KEY2,class T2, int (*hash2)(const KEY2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY2,T2,hash2>& m);



  private:
    class LN;

  public:
    class Iterator {
      public:
         typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashMap<T>
        ~Iterator();
        Entry       erase();
        std::string str  () const;
        HashMap<KEY,T,thash>::Iterator& operator ++ ();
        HashMap<KEY,T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        bool operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        Entry& operator *  () const;
        Entry* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashMap<KEY,T,thash>::begin () const;
        friend Iterator HashMap<KEY,T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor               current; //Bin Index & Cursor; stop: LN* == nullptr
        HashMap<KEY,T,thash>* ref_map;
        int                  expected_mod_count;
        bool                 can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
    public:
      LN ()                         : next(nullptr){}
      LN (const LN& ln)             : value(ln.value), next(ln.next){}
      LN (Entry v, LN* n = nullptr) : value(v), next(n){}

      Entry value;
      LN*   next;
  };

  int (*hash)(const KEY& k);  //Hashing function used (from template or constructor)
  LN** map      = nullptr;    //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;      //used/bins <= load_threshold
  int bins      = 1;          //# bins in array (start it at 1 so hash_compress doesn't % 0)
  int used      = 0;          //Cache for number of key->value pairs in the hash table
  int mod_count = 0;          //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const KEY& key)          const;  //hash function ranged to [0,bins-1]
  LN*   find_key             (int bin, const KEY& key) const;  //Returns reference to key's node or nullptr
  LN*   copy_list            (LN*   l)                 const;  //Copy the keys/values in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)       const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                   //Reallocate if load_factor > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);             //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};




////////////////////////////////////////////////////////////////////////////////
//
//HashMap class and related definitions

//Destructor/Constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::~HashMap() {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(double the_load_threshold, int (*chash)(const KEY& k))
: hash ( thash != nullptr ? thash : chash),	load_threshold(the_load_threshold)
{
	if (hash == nullptr)
		throw TemplateFunctionError("HashMap::default constructor nothing specified");
	if (thash != nullptr && chash != nullptr &&  thash != chash)
		throw TemplateFunctionError("HashMap::default constructor both specified and different");

	map = new LN*[bins]; //initialize your bin first
	for (int i =0 ; i< bins; i++)
		map[i] = new LN();
	//used/bins <= load_threshold
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(int initial_bins, double the_load_threshold, int (*chash)(const KEY& k))
: hash ( thash != nullptr ? thash : chash), load_threshold(the_load_threshold)
{
	if (hash == nullptr)
		throw TemplateFunctionError("HashMap::default constructor nothing specified");
	if (thash != nullptr && chash != nullptr &&  thash != chash)
		throw TemplateFunctionError("HashMap::default constructor both specified and different");

	map = new LN*[initial_bins]; //so, create your bins, in which a dbl ptr map points to an
	//array of Linked nodes!!!

	for (int i = 0 ; i < initial_bins; i++) // for these many bins
		map[i] = new LN(); 	//create a new linked node into each of them
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const HashMap<KEY,T,thash>& to_copy, double the_load_threshold, int (*chash)(const KEY& a))
{
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const std::initializer_list<Entry>& il, double the_load_threshold, int (*chash)(const KEY& k))
{
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template <class Iterable>
HashMap<KEY,T,thash>::HashMap(const Iterable& i, double the_load_threshold, int (*chash)(const KEY& k))
{
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::empty() const {
	return used == 0;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::size() const {
	return used;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_key (const KEY& key) const {
	return find_key(hash_compress(key), key);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_value (const T& value) const {
	/*
	 * bool find_value (const T& value) const; This method traverses all the LNs in all the bins in a hash table attempting to
	 *  to find any LN storing value: if successful it returns true; if unsuccessful it returns false
	 */
	for (int binNum = 0; binNum <bins; ++binNum)
		for (LN* node = map[binNum] ; node != nullptr; node = node->next)
			if (node->value.second == value)
				return true;
	return false;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::str() const {
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::put(const KEY& key, const T& value) {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::erase(const KEY& key) {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::clear() {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template<class Iterable>
int HashMap<KEY,T,thash>::put_all(const Iterable& i) {
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class KEY,class T, int (*thash)(const KEY& a)>
T& HashMap<KEY,T,thash>::operator [] (const KEY& key) {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
const T& HashMap<KEY,T,thash>::operator [] (const KEY& key) const {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>& HashMap<KEY,T,thash>::operator = (const HashMap<KEY,T,thash>& rhs) {
//	Don't just put each key->value pair in the map.

}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator == (const HashMap<KEY,T,thash>& rhs) const {
//	Don't just put each key->value pair in the map.


}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator != (const HashMap<KEY,T,thash>& rhs) const {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>& m) {
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::begin () const -> HashMap<KEY,T,thash>::Iterator {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::end () const -> HashMap<KEY,T,thash>::Iterator {
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::hash_compress (const KEY& key) const {
/*
 * int hash_compress (const KEY& key) const; This method uses the hash function supplied by the constructor and the number of bins
 * in the current hash table, to compute the bin index of any given key. Remember to compute the absolute value of the hash function
 * (which can return a negative result) and use the remainder operator (%) to ensure a bin index in the range [0,bins).
 */
//	int h_value = abs(hash(key)) % bins; //so we do have an absolute function, AND DON'T FORGET TO MOD IT.
//	std::cout<<"Here is the hash : "<< h_value<<"\n\n";
	return abs(hash(key)) % bins;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::find_key (int bin, const KEY& key) const {
	/*
	 * LN* find_key (int bin, const KEY& key) const; This method attempts to find the LN storing key in the bin index of a hash table:
	 *  if successful it returns a pointer to that LN (possibly to examine or update its associated value);
	 *   if unsuccessful it returns nullptr. The caller of this function will determine what to do with the pointer returned.
	 */
	for (LN* node = map[bin]; node != nullptr; node = node->next)
		if (node->value.first == key)
			return node;
	return nullptr; //same as last project

}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::copy_list (LN* l) const {
	/*
	 * LN* copy_list(LN* l) const; This method copies a linked list (which is a bin; actually,
	 * the values in the list can occur in any order), including the trailer node.
	 *  It is called by copy_hash_table
	 */
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN** HashMap<KEY,T,thash>::copy_hash_table (LN** ht, int bins) const {
	/*
	 * LN** copy_hash_table(LN** ht, int bins) const; This method copies an entire hash table,
	 * by allocating space for the bins and then copying the list nodes in each bin.
	 * It does not need to rehash/call hash_compress.
	 */
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::ensure_load_threshold(int new_used) {
	/*
	 * void ensure_load_factor(int new_used); This method ensures that a hash table with new_used values does
	 * not exceed the load factor (based on this value and the number of bins currently in the hash table).
	 *  If this ratio (compute it with doubles) would exceed the load factor threshold,
	 *  it creates a new hash table with twice the number of bins and puts all the values in the original hash table into
	 *  the new one (which requires rehashing each, since compression using a new number of bins can produce a different bin
	 *  index from hash_compress).
	 */
	double ratio = new_used/bins; //this is our current ratio;
	if (ratio <= load_threshold) //if the ratio <= threshold, don't make any changes
		return;
	LN** prev_map = map;	//so take all the old values
	T	 prev_bin = bins;

	bins = 2 * prev_bin;	//Create the new values
	map = new LN[bins];

	for (int i = 0 ; i < bins; i++)
		map[i] = new LN();	//assign new value again. We won't have any memory leaks because we will delete old value later

	// time to copy over values
	for (int prevNum = 0 ; prevNum< prev_bin; ++prevNum)
	{
		LN* prev_LN = prev_map[prevNum];//This is a node at bin index.
		for (LN* prev_node = prev_LN; prev_node != nullptr; prev_node = prev_node->next)
		{
			//need to use that for the new map index
			//need to create copy of LN value to transfer over
			//Need to make that copy.next point to a new LN()
			int hash_bin = hash_compress(prev_node->value.first);	//get our hash key
			LN* copying = prev_node;
			copying->next = map[hash_bin];
			map[hash_bin] = copying;
		}
		delete prev_LN;	//delete that pointer, which in this loop will delete all pointers within array
	}
	delete [] prev_map; //now delete the entire array. Can do because you cleared the previous one
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::delete_hash_table (LN**& ht, int bins) {
	/*
	 * void delete_hash_table(LN**& ht, int bins); This method deletes every LN used in the hash table (including trailer nodes),
	 *  and then deletes the hash table array itself.
	 */
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::Iterator::advance_cursors(){
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin)
: ref_map(iterate_over), expected_mod_count(ref_map->mod_count) {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::~Iterator()
{}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::Iterator::erase() -> Entry {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::Iterator::str() const {
}

template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ () -> HashMap<KEY,T,thash>::Iterator& {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ (int) -> HashMap<KEY,T,thash>::Iterator {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const {
//Don't just put each key->value pair in the map.


}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>& HashMap<KEY,T,thash>::Iterator::operator *() const {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>* HashMap<KEY,T,thash>::Iterator::operator ->() const {
}


}

#endif /* HASH_MAP_HPP_ */
