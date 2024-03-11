//
// Created by l50029536 on 2022/9/30.
//
//#define LOG

#include "arc_replacer.h"

RC ARCReplacer::access(const Key &key) {
    ts++;
    // case#1
    if ((lruList_t1_.count(key)) != 0 || (lruList_t2_.count(key)) != 0) {
        hit_count_++;
        if (lruList_t1_.count(key) != 0) {
            lruList_t1_.remove(key);
        } else {
            lruList_t2_.remove(key);
        }
        lruList_t2_.push_front(key);
        return RC::SUCCESS;
    }

    miss_count_++;
    // case#2
    if (lruList_b1_.count(key) != 0) {
        p_ = std::min(
            p_ + (lruList_b1_.size() >= lruList_b2_.size()
                      ? 1
                      : (double)lruList_b2_.size() / lruList_b1_.size()),
            (double)buffer_size_);
        replace_(key);
        lruList_b1_.remove(key);
        lruList_t2_.push_front(key);
        return RC::SUCCESS;
    }
    // case#3
    if (lruList_b2_.count(key) != 0) {
        p_ = std::max(
            p_ - (lruList_b2_.size() >= lruList_b1_.size()
                      ? 1
                      : (double)lruList_b1_.size() / lruList_b2_.size()),
            (double)0);
        replace_(key);
        lruList_b2_.remove(key);
        lruList_t2_.push_front(key);
        return RC::SUCCESS;
    }
    // case#4
    if (lruList_t1_.size() + lruList_b1_.size() == buffer_size_) {
        if (lruList_t1_.size() < buffer_size_) {
            lruList_b1_.pop_back();
            replace_(key);
        } else {
            lruList_t1_.pop_back();
        }
    } else {
        if (lruList_b1_.size() + lruList_t1_.size() + lruList_b2_.size() +
                lruList_t2_.size() >=
            buffer_size_) {
            if (lruList_b1_.size() + lruList_t1_.size() + lruList_b2_.size() +
                    lruList_t2_.size() ==
                2 * buffer_size_) {
                lruList_b2_.pop_back();
            }
            replace_(key);
        }
    }
    lruList_t1_.push_front(key);
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string ARCReplacer::get_name() { return std::string("ARC_CACHE_MANAGER"); }

RC ARCReplacer::check_consistency() {
    if (lruList_t1_.size() + lruList_t2_.size() > buffer_size_) {
        return RC::FAILED;
    }

    if (lruList_t1_.size() + lruList_t2_.size() + lruList_b1_.size() +
            lruList_b2_.size() >
        2 * buffer_size_) {
        return RC::FAILED;
    }
    if (p_ < 0 || p_ > buffer_size_) {
        return RC::FAILED;
    }
    return RC::SUCCESS;
}

RC ARCReplacer::replace_(const Key &key) {
    if ((lruList_t1_.size() != 0) &&
        ((lruList_t1_.size() > (int32_t)p_) ||
         (lruList_b1_.count(key) && lruList_t1_.size() == (int32_t)p_))) {
        Key old_key;
        lruList_t1_.pop_back(old_key);
        lruList_b1_.push_front(old_key);
    } else {
        Key old_key;
        lruList_t2_.pop_back(old_key);
        lruList_b2_.push_front(old_key);
    }
    return RC::SUCCESS;
}