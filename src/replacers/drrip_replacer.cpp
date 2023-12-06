//
// Created by Clouds on 2023/1/12.
//

#include "drrip_replacer.h"

RC DRRIPReplacer::access(const Key &key) {
    ts++;
    int new_bitmap;
    if (u_map_.count(key) == 0) {
        miss_count_++;
        if (u_map_.size() >= buffer_size_) {
            while (index_sets_[evict_bitmap_]->size() == 0) {
                auto temp = index_sets_[evict_bitmap_];
                for (int i = evict_bitmap_; i >= 1; i--) {
                    index_sets_[i] = std::move(index_sets_[i - 1]);
                }
                index_sets_[0] = temp;
            }
            auto item = index_sets_[evict_bitmap_]->begin();
            u_map_.erase(*item);
            index_sets_[evict_bitmap_]->erase(item);
        }
        if (rand() % 100 < 5) {
            new_bitmap = evict_bitmap_ - 2;
        } else {
            new_bitmap = evict_bitmap_ - 1;
        }
    } else {
        hit_count_++;
        auto iter = u_map_[key];
        for (auto index_set : index_sets_) {
            if (index_set->find(*iter) != index_set->end()) {
                index_set->erase(*iter);
                break;
            }
        }
        new_bitmap = 0;
    }
    //    std::cout << new_bitmap << ' ' << key << '\n';
    auto item = index_sets_[new_bitmap]->insert(key);
    u_map_[key] = item.first;
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string DRRIPReplacer::get_name() { return {"SRRIP"}; }

RC DRRIPReplacer::check_consistency() {
    if (u_map_.size() > buffer_size_) {
        return RC::FAILED;
    }
    return RC::UNIMPLEMENT;
}
