//
// Created by Clouds on 2023/9/20.
//
//#define LOG
#include "lirs_replacer.h"

RC LIRSReplacer::access(const Key &key) {
    ts++;
    if (map_.find(key) != map_.end()) {  // find it
                                         //        increase_hit_count();
        auto pnode = map_[key];
        if (!IS_VALID(pnode->value)) {
            ++used_size_;
        }
        Get(key);

        return RC::SUCCESS;
    }
    increase_miss_count();
    if (used_size_ >= cache_size_ || q_.size() >= q_size_) {  // 清理
        FreeOne();
    }

    // S is not FULL, so just input it as LIR
    lirs_node *p = new lirs_node(key, 0, s_.end(), q_.end());
    assert(p);
    Push(p, true);
    ++used_size_;

    // S is FULL, so just input it as HIR
    if (used_size_ > s_size_) {
        // if (s_.size() > s_size_) {
        p->type = HIR;
        Push(p, false);

        hirs_.push_front(p);
        p->hirs = hirs_.begin();
    }
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string LIRSReplacer::get_name() { return {"LIRS"}; }