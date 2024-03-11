//
// Created by Yunfan Li on 2022/9/28.
//

#ifndef HILL_CACHE_REPLACERS_LRU_REPLACER_H
#define HILL_CACHE_REPLACERS_LRUREPLACER_H

#include <unordered_map>

#include "def.h"
#include "link_list.h"
#include "lru_list.h"
#include "replacer.h"

class LRUReplacer : public Replacer {
   public:
    LRUReplacer(int32_t buffer_size, int32_t stats_interval)
        : Replacer(buffer_size, stats_interval) {}

    ~LRUReplacer() {}
    RC access(const Key &key) override;

    std::string get_name() override;

   private:
    LRUList<Key> lruList_;
};

#endif  // HILL_CACHE_REPLACERS_LRUREPLACER_H