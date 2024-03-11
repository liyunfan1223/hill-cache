//
// Created by l50029536 on 2022/9/30.
//

#pragma once

#include <unordered_map>

#include "link_list.h"
#include "lru_list.h"
#include "replacer.h"

class ARCReplacer : public Replacer {
   public:
    ARCReplacer(int32_t buffer_size, int32_t stats_interval)
        : Replacer(buffer_size, stats_interval) {
        p_ = 0;
    }

    ~ARCReplacer() {}
    RC access(const Key &key) override;

    std::string get_name() override;
    RC check_consistency() override;

   private:
    RC replace_(const Key &key);
    LRUList<Key> lruList_t1_, lruList_t2_, lruList_b1_, lruList_b2_;
    double p_;
};
