//
// Created by l50029536 on 2022/9/29.
//

#include "lfu_replacer.h"

RC LFUReplacer::access(const Key &key) {
    ts++;
    int32_t freq = 1;
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
        freq = item->freq + 1;
        u_map_.erase(item->key);
        buffer_set_.erase(item);
    }
    auto result = buffer_set_.insert(Status(freq, ts, key));
    u_map_[key] = result.first;
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string LFUReplacer::get_name() { return std::string("LFU"); }