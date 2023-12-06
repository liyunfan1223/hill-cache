//
// Created by Yunfan Li on 2023/2/9.
//

#include "lazy_update_heap.h"

void LUHeap::Push(Key key, double value = 1) {
    increase_ts();
    heap_.push_back(LUHeapEntry(key, value, ts_, ts_));
    index_[key] = heap_.size() - 1;
    heapify_up(heap_.size() - 1);
}

void LUHeap::Add(Key key, double value) {
    if (!InHeap(key)) {
        Push(key, value);
        return;
    }
    increase_ts();
    int32_t index = index_[key];
    update_value(index);
    heap_[index].value += value;
    heap_[index].ts_last_access = ts_;
    if (value >= 0) {
        heapify_down(index);
    } else {
        heapify_up(index);
    }
}

void LUHeap::Set(Key key, double value) {
    if (!InHeap(key)) {
        Push(key, value);
        return;
    }
    increase_ts();
    int32_t index = index_[key];
    heap_[index].value = value;

    heap_[index].ts_last_access = ts_;
    if (value >= 0) {
        heapify_down(index);
    } else {
        heapify_up(index);
    }
}

Key LUHeap::Pop() {
    Key ret_key = heap_[0].key;
    exchange(0, heap_.size() - 1);
    pop_back();
    heapify_down(0);
    return ret_key;
}

void LUHeap::heapify_up(int32_t index) {
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

void LUHeap::heapify_down(int32_t index) {
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

void LUHeap::update_value(int32_t index) {
    if (index >= heap_.size()) {
        return;
    }
    heap_[index].value *=
        pow(decay_ratio_, (ts_ - heap_[index].ts_last_update));
    heap_[index].ts_last_update = ts_;
}

int32_t LUHeap::get_smaller_son(int32_t index) {
    assert(!is_leaf(index));
    if (!has_lson(index)) {
        return get_rson(index);
    }
    if (!has_rson(index)) {
        return get_lson(index);
    }
    LUHeapEntry &left_son = heap_[get_lson(index)];
    LUHeapEntry &right_son = heap_[get_rson(index)];
    return left_son < right_son ? get_lson(index) : get_rson(index);
}

void LUHeap::pop_back() {
    index_.erase(heap_.back().key);
    heap_.pop_back();
}

void LUHeap::exchange(int32_t index_1, int32_t index_2) {
    std::swap(index_[heap_[index_1].key], index_[heap_[index_2].key]);
    std::swap(heap_[index_1], heap_[index_2]);
}
