//
// Created by Clouds on 2023/1/20.
//

#pragma once

#include "lazy_update_heap.h"
#include "replacer.h"

class EFSWReplacer : public Replacer {
   public:
    EFSWReplacer(int32_t buffer_size, int32_t stats_interval,
                 double half_life_ratio = 1.0, double miss_score = 1,
                 double hit_score = 10)
        : Replacer(buffer_size, stats_interval),
          // lu_heap_(log(0.5) / (half_life_ratio * (buffer_size))),
          luheap_(pow(0.5, (double)1.0 / (half_life_ratio * buffer_size))),
          // lu_heap_store_(log(0.5) / (half_life_ratio * (buffer_size)))
          luheap_store_(
              pow(0.5, (double)1.0 / (half_life_ratio * buffer_size))) {
        half_life_ratio_ = half_life_ratio;
        miss_score_ = miss_score;
        hit_score_ = hit_score;
    }

    ~EFSWReplacer() override = default;

    RC access(const Key &key) override;

    std::string get_name() override;

    std::string get_configuration() override;

    RC check_consistency() override;

   private:
    //    struct Status {
    //        Status(double score, Key key, int32_t timestamp)
    //                : score(score), key(key), timestamp(timestamp) {}
    //        double score;
    //        int32_t timestamp;
    //        Key key;
    //        bool operator < (const Status & rhs) const {
    //            if (fabs(score - rhs.score) > EPSILON) {
    //                return score < rhs.score;
    //            } else if (timestamp != rhs.timestamp) return timestamp <
    //            rhs.timestamp; return key < rhs.key;
    //        }
    //    };
    //    std::unordered_map<Key, double> score_;
    //    std::unordered_map<Key, int32_t> last_calc_ts_;
    //    std::set<Status> buffer_set_;
    //    std::unordered_map<Key, std::set<Status>::iterator> u_map_;
    //    int32_t timestamp_ = 0;
    //    double exponential_decay_ratio_;
    LUHeap luheap_;
    LUHeap luheap_store_;
    double miss_score_, hit_score_, half_life_ratio_;
};
