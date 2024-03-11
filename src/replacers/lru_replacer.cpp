//
// Created by Yunfan Li on 2022/9/28.
//

#include "lru_replacer.h"

RC LRUReplacer::access(const Key &key) {
    ts++;
    if (lruList_.count(key) == 0) {
        miss_count_ += 1;
        if (lruList_.size() == buffer_size_) {
            lruList_.pop_back();
        }
    } else {
        hit_count_ += 1;
        lruList_.remove(key);
    }
    lruList_.push_front(key);
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string LRUReplacer::get_name() { return {"LRU"}; }