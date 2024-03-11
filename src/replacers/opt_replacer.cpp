//
// Created by Yunfan Li on 2022/10/9.
//

#include "opt_replacer.h"

RC OPTReplacer::access(const Key &key) {
    ts++;
    key_access_u_map_[key].pop_front();
    if (u_map_.count(key) == 0) {
        miss_count_++;
        if (buffer_set_.size() >= buffer_size_) {
            auto item = buffer_set_.begin();
            u_map_.erase(item->key);
            buffer_set_.erase(item);
        }
    } else {
        hit_count_++;
        auto item = u_map_[key];
        buffer_set_.erase(item);
    }
    int32_t future_access_timestamp = key_access_u_map_[key].size
                                          ? key_access_u_map_[key].head->key
                                          : INT32_MAX;
    auto result = buffer_set_.insert(Status(future_access_timestamp, key));
    u_map_[key] = result.first;
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string OPTReplacer::get_name() { return {"OPT"}; }

RC OPTReplacer::check_consistency() { return Replacer::check_consistency(); }
