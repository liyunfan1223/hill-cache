//
// Created by Yunfan Li on 2023/2/12.
//

#ifndef HILL_CACHE_DYNAMIC_DECAY_HEAP_H
#define HILL_CACHE_DYNAMIC_DECAY_HEAP_H

#include "def.h"

class DDHeapEntry {
   public:
    DDHeapEntry(Key key, double value, int ts_last_access, int ts_last_update,
                int access_count)
        : key(key), value(value), access_count(access_count) {
        this->ts_last_update = 0;
        this->ts_last_access = ts_last_access;
        this->ts_last_update = ts_last_update;
    }
    Key key;
    double value;
    int32_t ts_last_access;
    int32_t ts_last_update;
    int32_t access_count;
    bool operator<(const DDHeapEntry &rhs) const {
        if (fabs(value - rhs.value) > EPSILON) {
            return value < rhs.value;
        }
        return ts_last_access < rhs.ts_last_access;
    }
};

class DDHeap {
   public:
    DDHeap() {}
    void Push(Key key, double value, double decay_ratio);
    void Add(Key key, double value, double decay_ratio);
    void Set(Key key, double value, double decay_ratio);
    double GetValue(Key key) {
        update_value(index_[key]);
        return heap_[index_[key]].value;
    }
    double GetAccessCount(Key key) {
        update_value(index_[key]);
        if (index_.find(key) != index_.end()) {
            return heap_[index_[key]].access_count;
        }
        return 0;
    }
    Key Pop();
    Key Peak() { return heap_[0].key; };
    int32_t Size() { return heap_.size(); }
    bool InHeap(Key key) { return index_.count(key); }
    void deep_copy(DDHeap &to_heap);
    void clear();

   private:
    int32_t get_father(int32_t index) { return index / 2; }
    void increase_ts(double decay_ratio);
    int32_t get_lson(const int32_t &index) const { return index * 2 + 1; }
    int32_t get_rson(const int32_t &index) const { return index * 2 + 2; }
    bool has_lson(int32_t index) { return get_lson(index) < heap_.size(); }
    bool has_rson(int32_t index) { return get_rson(index) < heap_.size(); }
    bool is_leaf(int32_t index) { return !has_lson(index) && !has_rson(index); }
    int32_t get_smaller_son(int32_t index);
    void update_value(int32_t index);
    void heapify_up(int32_t index);
    void heapify_down(int32_t index);
    void pop_back();
    void exchange(int32_t index_1, int32_t index_2);
    std::vector<DDHeapEntry> heap_;
    std::unordered_map<Key, int> index_;
    std::vector<double> decay_ratio_exp_suffix_sum_ = {0};
    int32_t ts_ = 0;
};

#endif  // HILL_CACHE_DYNAMIC_DECAY_HEAP_H
