//
// Created by Yunfan Li on 2022/9/28.
//

#ifndef HILL_CACHE_REPLACERS_REPLACER_H
#define HILL_CACHE_REPLACERS_REPLACER_H

#include "def.h"

class Replacer {
   public:
    explicit Replacer(int32_t buffer_size, int32_t stats_interval = -1)
        : buffer_size_(buffer_size), stats_interval(stats_interval) {
        if (stats_interval != -1) {
            enable_interval_stats_ = true;
        }
    }

    virtual ~Replacer() = default;

    std::string stats() {
        std::stringstream s;
        s << get_name() << ":"
          << " buffer_size:" << buffer_size_ << " hit_count:" << hit_count_
          << " miss_count:" << miss_count_ << " hit_rate:"
          << (float)hit_count_ / (float)(hit_count_ + miss_count_) * 100 << "\%"
          << std::endl;
        return s.str();
    }

    virtual RC access(const Key &key) = 0;
    virtual std::string get_name() = 0;
    virtual std::string get_configuration() { return {""}; }
    virtual RC check_consistency() { return RC::DEFAULT; }
    int32_t hit_count() const { return hit_count_; }
    int32_t miss_count() const { return miss_count_; }
    int32_t increase_hit_count() {
        hit_count_ += 1;
        return hit_count_;
    }
    int32_t increase_miss_count() {
        miss_count_ += 1;
        return miss_count_;
    }

   protected:
    const int32_t buffer_size_;
    int32_t hit_count_{};
    int32_t miss_count_{};
    bool enable_interval_stats_{false};
    int32_t stats_interval{};
    int ts{};
};

#endif  // HILL_CACHE_REPLACERS_REPLACER_H