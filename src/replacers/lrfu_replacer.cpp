//
// Created by Yunfan Li on 2023/2/9.
//

#include "lrfu_replacer.h"

RC LRFUReplacer::access(const Key &key) {
    ts++;
    if (lu_heap_.InHeap(key)) {
        hit_count_++;
    } else {
        miss_count_++;
        if (lu_heap_.Size() == buffer_size_) {
            lu_heap_.Pop();
        }
    }
    lu_heap_store_.Add(key, 1);
    if (lu_heap_.InHeap(key)) {
        lu_heap_.Add(key, 1);
    } else {
        lu_heap_.Add(key, lu_heap_store_.GetValue(key));
    }
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string LRFUReplacer::get_name() { return "LRFU"; }
