/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-2014 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class HashKey
// {
// public:
//    HashKey() {}
// 
//    size_t operator() () const { return 0; }
// 
//    bool operator == (const HashKey& k) const { return true; }
// 
// private:
// };
//
template <class HashKey, class HashData> class HashMap;

template <class HashKey, class HashData>
class HashMap
{
    typedef pair<HashKey, HashData> Data;

    class HashNode
    { 
        // TODO: design your own class!!
        friend class HashMap<HashKey, HashData>;
        friend class HashMap<HashKey, HashData>::iterator;

        HashNode(const Data& d,
                HashNode* prev=NULL,HashNode* next = NULL):
            _data(d),_prev(prev),_next(next) {}
        HashNode():_prev(NULL),_next(NULL){}

        Data     _data;
        HashNode* _prev;
        HashNode* _next;

    };

    public:
    HashMap() : _numBuckets(0), _buckets(0) {}
    HashMap(size_t b) : _numBuckets(0), _buckets(0) { init(b); }
    ~HashMap() { reset(); }

    // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
    // o An iterator should be able to go through all the valid HashNodes
    //   in the HashMap
    // o Functions to be implemented:
    //   - constructor(s), destructor
    //   - operator '*': return the HashNode
    //   - ++/--iterator, iterator++/--
    //   - operators '=', '==', !="
    //
    // (_bId, _bnId) range from (0, 0) to (_numBuckets, 0)
    //
    class iterator
    {
        friend class HashMap<HashKey, HashData>;

        public:
        iterator(HashNode* n= 0): _node(n) {}
        iterator(const iterator& i) : _node(i._node) {}
        ~iterator() {} // Should NOT delete _node

        // TODO: implement these overloaded operators
        const Data& operator * () const { return _node->_data; }
        Data& operator * () { return _node->_data; }
        iterator& operator ++ () {_node = _node->_next; return *(this); }
        iterator operator ++ (int) {
            iterator tmp = iterator(*this);
            _node = _node->_next;
            return tmp; 
        }
        iterator& operator -- () {_node = _node->_prev; return *(this); }
        iterator operator -- (int) {
            iterator tmp = iterator(*this);
            _node = _node->_prev;
            return tmp; 
        }

        iterator& operator = (const iterator& i) { _node=i._node;return *this;}
        bool operator != (const iterator& i) const{return (_node != i._node);}
        bool operator == (const iterator& i) const {return (_node == i._node);}


        private:
        HashNode* _node;
    };

    void init(size_t b) {
        reset(); _numBuckets = b; _buckets = new vector<HashNode* >[b]; }
    void reset() {
        _dummyEnd._prev = NULL;
        if (_buckets) {
            for(size_t i=0;i<_numBuckets;++i)
            {
                for(size_t j=0;j<_buckets[i].size();++j){
                    delete _buckets[i][j];}
            }
            _numBuckets = 0;
            delete [] _buckets; _buckets = 0;
        }
    }

    size_t numBuckets() const { return _numBuckets; }

    vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
    const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

    // TODO: implement these functions
    //
    // Point to the first valid data
    iterator begin() const
    {
        for (size_t i = 0;i<_numBuckets;++i) {
            if(!(_buckets[i].empty()))
                return _buckets[i].front();
        }
        return &_dummyEnd; 
    }
    // Pass the end
    iterator end() const {
        return &_dummyEnd; 
    }//the real end node 's next is NULL
    // return true if no valid data
    bool empty() const { return (begin() == end()); }
    // number of valid data
    size_t size() const
    {
        size_t ctr = 0;
        for(iterator it= begin();it!=end();++it){++ctr;}
        return ctr;
    }

    // check if d is in the hash...
    // if yes, return true;
    // else return false;
    bool check(const HashKey& key,size_t& matchId) const { 
        size_t bkNum = bucketNum(key); 
        for (size_t i = 0;i<_buckets[bkNum].size();++i) {
            if(key == _buckets[bkNum][i]->_data.first){
                matchId = _buckets[bkNum][i]->_data.second;
                return true;
            }
        }   
        return false;
    }



    // return true if inserted successfully (i.e. d is not in the hash)
    // return false is d is already in the hash ==> will not insert
    void insert(const Data& d)
    {
            size_t bkNum = bucketNum(d.first);
            HashNode* newNode = new HashNode(d);

            //set prev
            if(_buckets[bkNum].empty())
            {
                HashNode* prev = findPrev(newNode);
                newNode->_prev = prev;
                if(prev!=NULL)
                    prev->_next = newNode;
            }
            else
            {
                size_t size = _buckets[bkNum].size();
                HashNode* prev = _buckets[bkNum][size-1];
                newNode->_prev = prev;
                assert(prev!=NULL);
                prev->_next = newNode;
            }
            //set next
            HashNode* next = findNext(newNode);
            newNode->_next = next;
            next->_prev = newNode;

            _buckets[bkNum].push_back(newNode);
    }

    private:
    // Do not add any extra data member
    size_t            _numBuckets;
    vector<HashNode* >* _buckets;
    mutable HashNode _dummyEnd;
    /***Helper Function***/
    size_t bucketNum(const HashKey& key) const {
        return (key() % _numBuckets); }
    HashNode* findPrev(const HashNode* curNode)const
    {
        size_t curBk = bucketNum(curNode->_data.first);
        for (size_t i = 1;i<=curBk;++i) {
            if(!(_buckets[curBk-i].empty())){
                return (_buckets[curBk -i].back());
            }
        }
        return NULL;
    }
    HashNode* findNext(const HashNode* curNode)const
    {
        size_t curBk = bucketNum(curNode->_data.first);
        assert(curBk < _numBuckets);
        for (size_t i = curBk+1;i<_numBuckets;++i) {
            if(!(_buckets[i].empty())){
                return (_buckets[i].front());}
        }
        return &(_dummyEnd);
    }



};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
//template <class CacheKey, class CacheData>
//class Cache
//{
//typedef pair<CacheKey, CacheData> CacheNode;

//public:
//Cache() : _size(0), _cache(0) {}
//Cache(size_t s) : _size(0), _cache(0) { init(s); }
//~Cache() { reset(); }

//// NO NEED to implement Cache::iterator class

//// TODO: implement these functions
////
//// Initialize _cache with size s
//void init(size_t s) { }
//void reset() { }

//size_t size() const { return _size; }

//CacheNode& operator [] (size_t i) { return _cache[i]; }
//const CacheNode& operator [](size_t i) const { return _cache[i]; }

//// return false if cache miss
//bool read(const CacheKey& k, CacheData& d) const { return false; }
//// If k is already in the Cache, overwrite the CacheData
//void write(const CacheKey& k, const CacheData& d) { }

//private:
//// Do not add any extra data member
//size_t         _size;
//CacheNode*     _cache;
//};


#endif // MY_HASH_H
