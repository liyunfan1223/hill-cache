//
// Created by Yunfan Li on 2023/10/11.
//

#ifndef HILL_CACHE_DEQUE_DICT_HPP
#define HILL_CACHE_DEQUE_DICT_HPP

#include <list>
#include "def.h"


template<class V>
class DequeDict
{
    struct DequeEntry
    {
        Key key;
        V value;
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
            DequeEntry entry{key, value};
            list_.push_back(entry);
            k_map_[key] = (--list_.end());
        }
        void Remove(Key key) {
            assert(k_map_.find(key) != k_map_.end());
            list_.erase(k_map_[key]);
            k_map_.erase(key);
        }
        V PopFirst() {
            DequeEntry entry = list_.front();
            list_.pop_front();
            k_map_.erase(entry.key);
            return entry.value;
        }
        V GetFirst() {
            DequeEntry entry = list_.front();
            return entry.value;
        }
        void InplaceUpdate(Key key, V value) {
            assert(k_map_.find(key) != k_map_.end());
            k_map_[key]->value = value;
        }
        int32_t Size() { return k_map_.size(); }
    private:
        std::list<DequeEntry> list_;
        std::unordered_map<Key, typename std::list<DequeEntry>::iterator> k_map_;
};

#endif //HILL_CACHE_DEQUE_DICT_HPP
