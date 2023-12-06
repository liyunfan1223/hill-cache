//
// Created by Yunfan Li on 2023/2/12.
//

#include "dynamic_decay_lru.h"

void DDLru::Push(Key key, double value = 1, double decay_ratio = 0) {
    increase_ts(decay_ratio);
    //    if (list_size_ > 0 && index_.size() >= list_size_) {
    //        Pop();
    //    }
    list_.push_front(DDLruEntry(key, value, ts_, ts_));
    index_[key] = list_.begin();
}

void DDLru::Add(Key key, double value = 1, double decay_ratio = 0) {
    if (!InList(key)) {
        Push(key, value, decay_ratio);
        return;
    }
    increase_ts(decay_ratio);
    update_value(*index_[key]);
    double base_value = index_[key]->value;
    list_.erase(index_[key]);
    list_.push_front(DDLruEntry(key, base_value + value, ts_, ts_));
    index_[key] = list_.begin();
}

Key DDLru::Pop() {
    Key ret_key = list_.rbegin()->key;
    index_.erase(ret_key);
    list_.pop_back();
    return ret_key;
}

void DDLru::update_value(DDLruEntry &entry) {
    entry.value *= exp(decay_ratio_exp_suffix_sum_[ts_] -
                       decay_ratio_exp_suffix_sum_[entry.ts_last_update]);
    // pow(decay_ratio_exp_suffix_sum_, (ts_ - list_[index].ts_last_update));
    entry.ts_last_update = ts_;
}

void DDLru::pop_back() {
    index_.erase(list_.back().key);
    list_.pop_back();
}

void DDLru::increase_ts(double decay_ratio) {
    ts_++;
    double last = decay_ratio_exp_suffix_sum_.back();
    decay_ratio_exp_suffix_sum_.push_back(last + decay_ratio);
}