#ifndef BST_MAP_HPP_
#define BST_MAP_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"
#include "array_queue.hpp"   //For traversal


namespace ics {


//Instantiate such that tlt(a,b) is true, iff a is in the left subtree rooted by b
//With a tlt specified in the template, the constructor cannot specify a clt.
//If a tlt is defaulted, then the constructor must supply a clt (they cannot both be nullptr)
template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b) = nullptr> class BSTMap {
  public:
    typedef pair<KEY,T> Entry;

    //Destructor/Constructors
    ~BSTMap();

    BSTMap          (bool (*clt)(const KEY& a, const KEY& b) = nullptr);
    BSTMap          (const BSTMap<KEY,T,tlt>& to_copy, bool (*clt)(const KEY& a, const KEY& b) = nullptr);
    explicit BSTMap (const std::initializer_list<Entry>& il, bool (*clt)(const KEY& a, const KEY& b) = nullptr);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit BSTMap (const Iterable& i, bool (*clt)(const KEY& a, const KEY& b) = nullptr);


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
    BSTMap<KEY,T,tlt>& operator = (const BSTMap<KEY,T,tlt>& rhs);
    bool operator == (const BSTMap<KEY,T,tlt>& rhs) const;
    bool operator != (const BSTMap<KEY,T,tlt>& rhs) const;

    template<class KEY2,class T2, bool (*lt2)(const KEY2& a, const KEY2& b)>
    friend std::ostream& operator << (std::ostream& outs, const BSTMap<KEY2,T2,lt2>& m);



    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of BSTMap<T>
        ~Iterator();
        Entry       erase();
        std::string str  () const;
        BSTMap<KEY,T,tlt>::Iterator& operator ++ ();
        BSTMap<KEY,T,tlt>::Iterator  operator ++ (int);
        bool operator == (const BSTMap<KEY,T,tlt>::Iterator& rhs) const;
        bool operator != (const BSTMap<KEY,T,tlt>::Iterator& rhs) const;
        Entry& operator *  () const;
        Entry* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const BSTMap<KEY,T,tlt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator BSTMap<KEY,T,tlt>::begin () const;
        friend Iterator BSTMap<KEY,T,tlt>::end   () const;

      private:
        //If can_erase is false, the value has been removed from "it" (++ does nothing)
        ArrayQueue<Entry> it;                 //Queue of all associations (from begin), to use as iterator via dequeue
        BSTMap<KEY,T,tlt>* ref_map;
        int               expected_mod_count;
        bool              can_erase = true;

        //Called in friends begin/end
        Iterator(BSTMap<KEY,T,tlt>* iterate_over, bool from_begin);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class TN {
      public:
        TN ()                     : left(nullptr), right(nullptr){}
        TN (const TN& tn)         : value(tn.value), left(tn.left), right(tn.right){}
        TN (Entry v, TN* l = nullptr,
                     TN* r = nullptr) : value(v), left(l), right(r){}

        Entry value;
        TN*   left;
        TN*   right;
    };

  bool (*lt) (const KEY& a, const KEY& b); // The lt used for searching BST (from template or constructor)
  TN* map       = nullptr;
  int used      = 0;                       //Cache for number of key->value pairs in the BST
  int mod_count = 0;                       //For sensing concurrent modification

  //Helper methods (find_key written iteratively, the rest recursively)
  TN*   find_key            (TN*  root, const KEY& key)                 const; //Returns reference to key's node or nullptr
  bool  has_value           (TN*  root, const T& value)                 const; //Returns whether value is is root's tree
  TN*   copy                (TN*  root)                                 const; //Copy the keys/values in root's tree (identical structure)
  void  copy_to_queue       (TN* root, ArrayQueue<Entry>& q)            const; //Fill queue with root's tree value
  bool  equals              (TN*  root, const BSTMap<KEY,T,tlt>& other) const; //Returns whether root's keys/value are all in other
  std::string string_rotated(TN* root, std::string indent)              const; //Returns string representing root's tree

  T     insert              (TN*& root, const KEY& key, const T& value);       //Put key->value, returning key's old value (or new one's, if key absent)
  T&    find_addempty       (TN*& root, const KEY& key);                       //Return reference to key's value (adding key->T() first, if key absent)
  Entry remove_closest      (TN*& root);                                       //Helper for remove
  T     remove              (TN*& root, const KEY& key);                       //Remove key->value from root's tree
  void  delete_BST          (TN*& root);                                       //Deallocate all TN in tree; root == nullptr
};




////////////////////////////////////////////////////////////////////////////////
//
//BSTMap class and related definitions

//Destructor/Constructors

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::~BSTMap() {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::BSTMap(bool (*clt)(const KEY& a, const KEY& b))
{
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::BSTMap(const BSTMap<KEY,T,tlt>& to_copy, bool (*clt)(const KEY& a, const KEY& b))
{
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::BSTMap(const std::initializer_list<Entry>& il, bool (*clt)(const KEY& a, const KEY& b))
{
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
template <class Iterable>
BSTMap<KEY,T,tlt>::BSTMap(const Iterable& i, bool (*clt)(const KEY& a, const KEY& b))
{
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::empty() const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
int BSTMap<KEY,T,tlt>::size() const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::has_key (const KEY& key) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::has_value (const T& value) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
std::string BSTMap<KEY,T,tlt>::str() const {
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T BSTMap<KEY,T,tlt>::put(const KEY& key, const T& value) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T BSTMap<KEY,T,tlt>::erase(const KEY& key) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
void BSTMap<KEY,T,tlt>::clear() {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
template<class Iterable>
int BSTMap<KEY,T,tlt>::put_all(const Iterable& i) {
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T& BSTMap<KEY,T,tlt>::operator [] (const KEY& key) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
const T& BSTMap<KEY,T,tlt>::operator [] (const KEY& key) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>& BSTMap<KEY,T,tlt>::operator = (const BSTMap<KEY,T,tlt>& rhs) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::operator == (const BSTMap<KEY,T,tlt>& rhs) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::operator != (const BSTMap<KEY,T,tlt>& rhs) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
std::ostream& operator << (std::ostream& outs, const BSTMap<KEY,T,tlt>& m) {
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
auto BSTMap<KEY,T,tlt>::begin () const -> BSTMap<KEY,T,tlt>::Iterator {
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
typename BSTMap<KEY,T,tlt>::TN* BSTMap<KEY,T,tlt>::find_key (TN* root, const KEY& key) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::has_value (TN* root, const T& value) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
typename BSTMap<KEY,T,tlt>::TN* BSTMap<KEY,T,tlt>::copy (TN* root) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
void BSTMap<KEY,T,tlt>::copy_to_queue (TN* root, ArrayQueue<Entry>& q) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::equals (TN* root, const BSTMap<KEY,T,tlt>& other) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
std::string BSTMap<KEY,T,tlt>::string_rotated(TN* root, std::string indent) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T BSTMap<KEY,T,tlt>::insert (TN*& root, const KEY& key, const T& value) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T& BSTMap<KEY,T,tlt>::find_addempty (TN*& root, const KEY& key) {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
pair<KEY,T> BSTMap<KEY,T,tlt>::remove_closest(TN*& root) {
  if (root->right != nullptr)
    return remove_closest(root->right);
  else{
    Entry to_return = root->value;
    TN* to_delete = root;
    root = root->left;
    delete to_delete;
    return to_return;
  }
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
T BSTMap<KEY,T,tlt>::remove (TN*& root, const KEY& key) {
  if (root == nullptr) {
    std::ostringstream answer;
    answer << "BSTMap::erase: key(" << key << ") not in Map";
    throw KeyError(answer.str());
  }else
    if (key == root->value.first) {
      T to_return = root->value.second;
      if (root->left == nullptr) {
        TN* to_delete = root;
        root = root->right;
        delete to_delete;
      }else if (root->right == nullptr) {
        TN* to_delete = root;
        root = root->left;
        delete to_delete;
      }else
        root->value = remove_closest(root->left);
      return to_return;
    }else
      return remove( (lt(key,root->value.first) ? root->left : root->right), key);
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
void BSTMap<KEY,T,tlt>::delete_BST (TN*& root) {
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::Iterator::Iterator(BSTMap<KEY,T,tlt>* iterate_over, bool from_begin)
{
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
BSTMap<KEY,T,tlt>::Iterator::~Iterator()
{}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
auto BSTMap<KEY,T,tlt>::Iterator::erase() -> Entry {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
std::string BSTMap<KEY,T,tlt>::Iterator::str() const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
auto  BSTMap<KEY,T,tlt>::Iterator::operator ++ () -> BSTMap<KEY,T,tlt>::Iterator& {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
auto BSTMap<KEY,T,tlt>::Iterator::operator ++ (int) -> BSTMap<KEY,T,tlt>::Iterator {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::Iterator::operator == (const BSTMap<KEY,T,tlt>::Iterator& rhs) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
bool BSTMap<KEY,T,tlt>::Iterator::operator != (const BSTMap<KEY,T,tlt>::Iterator& rhs) const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
pair<KEY,T>& BSTMap<KEY,T,tlt>::Iterator::operator *() const {
}


template<class KEY,class T, bool (*tlt)(const KEY& a, const KEY& b)>
pair<KEY,T>* BSTMap<KEY,T,tlt>::Iterator::operator ->() const {
}


}

#endif /* BST_MAP_HPP_ */
