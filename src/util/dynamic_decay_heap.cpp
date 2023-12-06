//
// Created by Yunfan Li on 2023/2/12.
//

#include "dynamic_decay_heap.h"

void DDHeap::Push(Key key, double value = 1, double decay_ratio = 0) {
    increase_ts(decay_ratio);
    heap_.push_back(DDHeapEntry(key, value, ts_, ts_, 1));
    index_[key] = heap_.size() - 1;
    heapify_up(heap_.size() - 1);
}

void DDHeap::Add(Key key, double value = 1, double decay_ratio = 0) {
    if (!InHeap(key)) {
        Push(key, value, decay_ratio);
        return;
    }
    increase_ts(decay_ratio);
    int32_t index = index_[key];
    update_value(index);
    heap_[index].value += value;
    heap_[index].ts_last_access = ts_;
    heap_[index].access_count += 1;
    if (value >= 0) {
        heapify_down(index);
    } else {
        heapify_up(index);
    }
}

void DDHeap::Set(Key key, double value, double decay_ratio) {
    if (!InHeap(key)) {
        Push(key, value, decay_ratio);
        return;
    }
    increase_ts(decay_ratio);
    int32_t index = index_[key];
    heap_[index].value = value;

    heap_[index].ts_last_access = ts_;
    if (value >= 0) {
        heapify_down(index);
    } else {
        heapify_up(index);
    }
}

Key DDHeap::Pop() {
    Key ret_key = heap_[0].key;
    exchange(0, heap_.size() - 1);
    pop_back();
    heapify_down(0);
    return ret_key;
}

inline void DDHeap::heapify_up(int32_t index) {
    if (index == 0) {
        return;
    }
    int32_t father_index = get_father(index);
    update_value(father_index);
    if (heap_[index] < heap_[father_index]) {
        exchange(index, father_index);
        heapify_up(father_index);
    }
}

inline void DDHeap::heapify_down(int32_t index) {
    if (is_leaf(index)) {
        return;
    }
    update_value(get_lson(index));
    update_value(get_rson(index));
    int32_t smaller_son = get_smaller_son(index);
    if (heap_[smaller_son] < heap_[index]) {
        exchange(index, smaller_son);
        heapify_down(smaller_son);
    }
}

void DDHeap::update_value(int32_t index) {
    if (index >= heap_.size()) {
        return;
    }
    heap_[index].value *=
        exp(decay_ratio_exp_suffix_sum_[ts_] -
            decay_ratio_exp_suffix_sum_[heap_[index].ts_last_update]);
    // pow(decay_ratio_exp_suffix_sum_, (ts_ - list_[index].ts_last_update));
    heap_[index].ts_last_update = ts_;
}

int32_t DDHeap::get_smaller_son(int32_t index) {
    //    assert(!is_leaf(index));
    if (!has_lson(index)) {
        return get_rson(index);
    }
    if (!has_rson(index)) {
        return get_lson(index);
    }
    DDHeapEntry &left_son = heap_[get_lson(index)];
    DDHeapEntry &right_son = heap_[get_rson(index)];
    return left_son < right_son ? get_lson(index) : get_rson(index);
}

void DDHeap::pop_back() {
    index_.erase(heap_.back().key);
    heap_.pop_back();
}

void DDHeap::exchange(int32_t index_1, int32_t index_2) {
    std::swap(index_[heap_[index_1].key], index_[heap_[index_2].key]);
    std::swap(heap_[index_1], heap_[index_2]);
}

void DDHeap::increase_ts(double decay_ratio) {
    ts_++;
    double last = decay_ratio_exp_suffix_sum_.back();
    decay_ratio_exp_suffix_sum_.push_back(last + decay_ratio);
}

void DDHeap::deep_copy(DDHeap &to_heap) {
    to_heap.clear();
    to_heap.heap_.assign(heap_.begin(), heap_.end());
    to_heap.index_.insert(index_.begin(), index_.end());
    to_heap.decay_ratio_exp_suffix_sum_.assign(
        decay_ratio_exp_suffix_sum_.begin(), decay_ratio_exp_suffix_sum_.end());
    to_heap.ts_ = ts_;
}

void DDHeap::clear() {
    heap_.clear();
    index_.clear();
    decay_ratio_exp_suffix_sum_.clear();
    ts_ = 0;
}