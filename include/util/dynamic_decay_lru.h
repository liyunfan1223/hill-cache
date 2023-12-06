//
// Created by Yunfan Li on 2023/2/15.
//

#ifndef HILL_CACHE_DYNAMIC_DECAY_LRU_H
#define HILL_CACHE_DYNAMIC_DECAY_LRU_H

#include "def.h"
class DDLruEntry {
   public:
    DDLruEntry(Key key, double value, int ts_last_access, int ts_last_update)
        : key(key), value(value) {
        this->ts_last_update = 0;
        this->ts_last_access = ts_last_access;
        this->ts_last_update = ts_last_update;
    }
    Key key;
    double value;
    int32_t ts_last_access;
    int32_t ts_last_update;
    bool operator<(const DDLruEntry &rhs) const {
        if (fabs(value - rhs.value) > EPSILON) {
            return value < rhs.value;
        }
        return ts_last_access < rhs.ts_last_access;
    }
};

class DDLru {
   public:
    DDLru(int32_t list_size) : list_size_(list_size) {}
    void Push(Key key, double value, double decay_ratio);
    void Add(Key key, double value, double decay_ratio);
    double GetValue(Key key) {
        update_value(*index_[key]);
        return index_[key]->value;
    }
    Key Pop();
    Key Peak() { return list_.begin()->key; };
    int32_t Size() { return list_.size(); }
    bool InList(Key key) { return index_.count(key); }

   private:
    void increase_ts(double decay_ratio);
    void update_value(DDLruEntry &entry);
    void pop_back();
    int32_t list_size_;
    std::list<DDLruEntry> list_;
    std::unordered_map<Key, std::list<DDLruEntry>::iterator> index_;
    std::vector<double> decay_ratio_exp_suffix_sum_ = {0};
    int32_t ts_ = 0;
};

#endif  // HILL_CACHE_DYNAMIC_DECAY_LRU_H
