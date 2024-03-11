//
// Created by Clouds on 2023/9/20.
//
//#define LOG
#include "dlirs_replacer.h"

RC DLIRSReplacer::access(const Key &key) {
    ts++;
    bool miss = false;
    if (lirs.Contain(key)) {
        auto x = lirs.Get(key);
        if (x.is_lir) {
            hitLIR(key);
        } else {
            miss = hitHIRinLIRS(key);
        }
    } else if (q.Contain(key)) {
        hitHIRinQ(key);
    } else {
        miss = true;
        Miss(key);
    }
    if (miss) {
        increase_miss_count();
    } else {
        increase_hit_count();
    }
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string DLIRSReplacer::get_name() { return {"DLIRS"}; }