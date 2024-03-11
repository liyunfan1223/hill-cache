//
// Created by Yunfan Li on 2023/2/9.
//

#pragma once

#include "def.h"

class LUHeapEntry {
   public:
    LUHeapEntry(Key key, double value, int ts_last_access, int ts_last_update)
        : key(key), value(value) {
        this->ts_last_update = 0;
        this->ts_last_access = ts_last_access;
        this->ts_last_update = ts_last_update;
    }
    Key key;
    double value;
    int32_t ts_last_access;
    int32_t ts_last_update;
    bool operator<(const LUHeapEntry &rhs) const {
        if (fabs(value - rhs.value) > EPSILON) {
            return value < rhs.value;
        }
        return ts_last_access < rhs.ts_last_access;
    }
};

class LUHeap {
   public:
    LUHeap(double decay_ratio = 0.5) : decay_ratio_(decay_ratio) {}
    void Push(Key key, double value);
    void Add(Key key, double value);
    void Set(Key key, double value);
    double GetValue(Key key) {
        update_value(index_[key]);
        return heap_[index_[key]].value;
    }
    Key Pop();
    Key Peak() { return heap_[0].key; };
    int32_t Size() { return heap_.size(); }
    bool InHeap(Key key) { return index_.count(key); }

   private:
    int32_t get_father(int32_t index) { return index / 2; }
    void increase_ts() { ts_++; }
    int32_t get_lson(int32_t index) { return index * 2 + 1; }
    int32_t get_rson(int32_t index) { return index * 2 + 2; }
    bool has_lson(int32_t index) { return get_lson(index) < heap_.size(); }
    bool has_rson(int32_t index) { return get_rson(index) < heap_.size(); }
    bool is_leaf(int32_t index) { return !has_lson(index) && !has_rson(index); }
    int32_t get_smaller_son(int32_t index);
    void update_value(int32_t index);
    void heapify_up(int32_t index);
    void heapify_down(int32_t index);
    void pop_back();
    void exchange(int32_t index_1, int32_t index_2);
    std::vector<LUHeapEntry> heap_;
    std::unordered_map<Key, int> index_;
    double decay_ratio_;
    int32_t ts_;
};
