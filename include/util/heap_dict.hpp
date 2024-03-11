//
// Created by Yunfan Li on 2023/10/11.
//

#ifndef HILL_CACHE_HEAP_DICT_HPP
#define HILL_CACHE_HEAP_DICT_HPP

#include <list>
#include "def.h"


template<class V>
class HeapDict
{
    struct HeapEntry
    {
        Key key;
        V value;
        bool operator < (const HeapEntry &rhs) const {
            return value < rhs.value;
        }
    };
    public:
        bool Contain(Key key) {
            return k_map_.find(key) != k_map_.end();
        }
        V Get(Key key) {
            return k_map_[key]->value;
        }
        void Set(Key key, V value) {
            if (k_map_.find(key) != k_map_.end()) {
                Update(key, value);
            } else {
                Push(key, value);
            }
        }
        void Update(Key key, V value) {
            Remove(key);
            Push(key, value);
        }
        void Push(Key key, V value) {
            assert(k_map_.find(key) == k_map_.end());
            HeapEntry entry{key, value};
            typename std::set<HeapEntry>::iterator iter = set_.insert(entry).first;
            k_map_[key] = iter;
        }
        void Remove(Key key) {
            assert(k_map_.find(key) != k_map_.end());
            set_.erase(k_map_[key]);
            k_map_.erase(key);
        }
        V GetMin() {
            return set_.begin()->value;
        }
    private:
        std::set<HeapEntry> set_;
        std::unordered_map<Key, typename std::set<HeapEntry>::iterator> k_map_;
};

#endif //HILL_CACHE_HEAP_DICT_HPP
