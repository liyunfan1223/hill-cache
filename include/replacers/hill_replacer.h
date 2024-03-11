//
// Created by Yunfan Li on 2023/5/23.
//

#ifndef HILL_CACHE_HILL_REPLACER_H
#define HILL_CACHE_HILL_REPLACER_H

#include "def.h"
#include "replacer.h"

class HillReplacer;
class HillSubReplacer {
    struct HillEntry {
        HillEntry() = default;
        HillEntry(std::list<Key>::iterator key_iter, int insert_level,
                  int insert_ts, bool h_recency) {
            this->key_iter = key_iter;
            this->insert_level = insert_level;
            this->insert_ts = insert_ts;
            this->h_recency = h_recency;
        }

        std::list<Key>::iterator key_iter;
        int insert_level{};
        int insert_ts{};
        bool h_recency{};  // 高时近性 说明在顶层LRU中
    };

   public:
    friend HillReplacer;
    HillSubReplacer(int32_t size, double init_half, double hit_points,
                    int max_points_bits, double ghost_size_ratio,
                    double top_ratio, int32_t mru_threshold)
        : size_(size),
          init_half_(init_half),
          hit_points_(hit_points),
          max_points_bits_(max_points_bits),
          ghost_size_ratio_(ghost_size_ratio),
          top_ratio_(top_ratio),
          mru_threshold_(mru_threshold) {
        cur_half_ = init_half_;
        max_points_ = (1 << max_points_bits_) - 1;
        ghost_size_ = size_ * ghost_size_ratio;
        min_level_non_empty_ = max_points_;
        real_lru_.resize(max_points_);
        UpdateHalf(init_half_);
        lru_size_ = std::max(1, (int)(top_ratio_ * size));
        ml_size_ = size_ - lru_size_;
        if (mru_threshold_ == 0) {
            mru_threshold_ = -1;
        }
    }
    int Access(Key key) {
        bool hit = true;
        int32_t inserted_level = hit_points_;
        auto rm_iter = real_map_.find(key);
        if (rm_iter == real_map_.end()) {
            // miss
            hit = false;
            interval_miss_count_++;
            if (real_map_.size() == size_) {
                Evict();
            }
            auto gm_iter = ghost_map_.find(key);
            if (gm_iter != ghost_map_.end()) {
                // use level in ghost
                std::list<Key>::iterator hit_iter = gm_iter->second.key_iter;
                int level = GetCurrentLevel(gm_iter->second);
                // erase key in ghost
                ghost_lru_.erase(hit_iter);
                ghost_map_.erase(key);
                inserted_level += level;
            }
        } else {
            // hit
            interval_hit_count_++;
            if (!rm_iter->second.h_recency) {
                h1++;
                std::list<Key>::iterator hit_iter = rm_iter->second.key_iter;
                int level = GetCurrentLevel(rm_iter->second);
                // erase key in real, use level in real
                real_lru_[level].erase(hit_iter);
                real_map_.erase(rm_iter);
                inserted_level += level;
                while (real_lru_[min_level_non_empty_].empty() &&
                       min_level_non_empty_ < max_points_) {
                    min_level_non_empty_++;
                }
            } else {
                h2++;
                inserted_level += rm_iter->second.insert_level;
                top_lru_.erase(rm_iter->second.key_iter);
                top_lru_size_--;
                real_map_.erase(rm_iter);
            }
        }
        inserted_level = std::min(inserted_level, max_points_ - 1);

        if (top_lru_size_ >= lru_size_) {
            Key _key = top_lru_.back();
            auto &rm_entry = real_map_[_key];
            int lvl = rm_entry.insert_level;
            real_lru_[lvl].push_front(_key);
            real_map_[_key] =
                HillEntry(real_lru_[lvl].begin(), lvl, cur_ts_, false);
            if (lvl < min_level_non_empty_) {
                min_level_non_empty_ = lvl;
            }
            top_lru_.pop_back();
            top_lru_size_--;
        }
        top_lru_.push_front(key);
        top_lru_size_++;
        real_map_[key] =
            HillEntry(top_lru_.begin(), inserted_level, cur_ts_, true);
        cur_ts_++;
        if (cur_ts_ >= next_rolling_ts_) {
            Rolling();
        }
        return hit;
    }
    void Evict() {
        // evict key in real
        Key evict_key;
        int evict_level;
        int mx_ts = 0, sum = 0;
        bool front = false;
        if (min_level_non_empty_ <= mru_threshold_) {
            front = true;
            for (int i = min_level_non_empty_; i < max_points_; i++) {
                if (real_lru_[i].empty()) {
                    continue;
                }
                // MRU
                if (real_map_[real_lru_[i].front()].insert_ts > mx_ts) {
                    mx_ts = real_map_[real_lru_[i].front()].insert_ts;
                    evict_key = real_lru_[i].front();
                    evict_level = i;
                }
                sum += real_lru_[i].size();
                break;
            }
        } else {
            evict_key = real_lru_[min_level_non_empty_].back();
            evict_level = min_level_non_empty_;
        }
        if (front) {
            real_lru_[evict_level].pop_front();
        } else {
            real_lru_[evict_level].pop_back();
        }
        real_map_.erase(evict_key);
        // move to ghost
        if (ghost_size_ != 0 && evict_level != 0) {
            // evict ghost
            if (ghost_map_.size() >= ghost_size_) {
                Key evict_key = ghost_lru_.back();
                ghost_lru_.pop_back();
                ghost_map_.erase(evict_key);
            }
            // min_level_non_empty_ == evict_key's level
            ghost_lru_.push_front(evict_key);
            ghost_map_[evict_key] =
                HillEntry(ghost_lru_.begin(), evict_level, cur_ts_, false);
        }
        while (real_lru_[min_level_non_empty_].empty()) {
            min_level_non_empty_++;
        }
    }
    void Rolling() {
        for (int i = 1; i < max_points_; i++) {
            if (!real_lru_[i].empty()) {
                if (mru_threshold_ != -1) {
                    real_lru_[i / 2].splice(real_lru_[i / 2].end(),
                                            real_lru_[i]);
                } else {
                    real_lru_[i / 2].splice(real_lru_[i / 2].begin(),
                                            real_lru_[i]);
                }
                if (!real_lru_[i / 2].empty()) {
                    min_level_non_empty_ =
                        std::min(min_level_non_empty_, i / 2);
                }
            }
        }
        next_rolling_ts_ = cur_ts_ + cur_half_ * size_;
        prev_rolling_ts_ = cur_ts_;
        rolling_ts.push_back(cur_ts_);
        if (rolling_ts.size() > max_points_bits_) {
            rolling_ts.pop_front();
        }
    }
    void UpdateHalf(double cur_half) {
        cur_half_ = cur_half;
        if (cur_half_ < (double)1 / size_) {
            cur_half_ = (double)1 / size_;
        }
        if (cur_half_ > 1e14 / size_) {
            cur_half_ = 1e14 / size_;
        }
        next_rolling_ts_ = prev_rolling_ts_ + (int)(cur_half_ * size_);
    }
    void ReportAndClear(int32_t &miss_count,
                        int32_t &hit_count /*, int32_t &hit_top*/) {
        miss_count = interval_miss_count_;
        hit_count = interval_hit_count_;
        //        hit_top = interval_hit_top_;
        interval_miss_count_ = 0;
        interval_hit_count_ = 0;
        interval_hit_top_ = 0;
    }
    double GetCurHalf() const { return cur_half_; }
    int h1{}, h2{};  // debug
   private:
    int32_t GetCurrentLevel(const HillEntry &status) {
        int32_t est_level = status.insert_level;
        if (rolling_ts.empty()) {
            return est_level;
        }
        auto iter = rolling_ts.begin();
        for (int i = 0; i < rolling_ts.size(); i++) {
            if (status.insert_ts < *iter) {
                est_level >>= rolling_ts.size() - i;
                break;
            }
            iter++;
        }
        return est_level;
    }
    int32_t size_;
    double init_half_;  // 初始半衰期系数
    double cur_half_;

   private:
    // 当前半衰期系数
    int32_t lru_size_;         // LRU部分大小
    int32_t ml_size_;          // RGC部分大小
    double hit_points_;        // 命中后得分
    int32_t max_points_bits_;  // 最高得分为 (1 << max_points_bits_) - 1
    int32_t max_points_;       // 最高得分
    int32_t min_level_non_empty_;  // 当前最小的得分
    double ghost_size_ratio_;      // 虚缓存大小比例
    int32_t ghost_size_;
    int32_t interval_hit_count_ = 0;       // 统计区间命中次数
    int32_t interval_miss_count_ = 0;      // 统计区间为命中次数
    int32_t interval_hit_top_ = 0;         // 在top上命中次数
    int32_t next_rolling_ts_ = INT32_MAX;  // 下一次滚动的时间戳
    int32_t cur_ts_ = 0;                   // 当前时间戳
    int32_t prev_rolling_ts_ = 0;
    std::vector<std::list<Key>> real_lru_;
    std::list<Key> ghost_lru_;
    std::list<Key> top_lru_;
    std::unordered_map<Key, HillEntry> real_map_, ghost_map_;
    std::list<int32_t> rolling_ts;
    double top_ratio_;
    int32_t mru_threshold_;
    int32_t top_lru_size_ = 0;
};

class HillReplacer : public Replacer {
   public:
    HillReplacer(int32_t buffer_size, int32_t stats_interval,
                 double init_half = 16.0f, double hit_point = 1.0f,
                 int32_t max_points_bits = 6, double ghost_size_ratio = 4.0f,
                 double lambda = 1.0f, double simulator_ratio = 0.67f,
                 double top_ratio = 0.05f, double delta_bound = 0.01f,
                 bool update_equals_size = true, int32_t mru_threshold = 64,
                 int32_t minimal_update_size = 10000)
        : Replacer(buffer_size, stats_interval),
          replacer_r_(buffer_size, init_half, hit_point, max_points_bits,
                      ghost_size_ratio, top_ratio, mru_threshold),
          replacer_s_(buffer_size, init_half * simulator_ratio, hit_point,
                      max_points_bits, ghost_size_ratio, top_ratio,
                      mru_threshold),
          lambda_(lambda),
          init_half_(init_half),
          simulator_ratio_(simulator_ratio),
          delta_bound_(delta_bound) {
        update_interval_ = std::max(100, buffer_size);
        update_interval_ = std::min(minimal_update_size, buffer_size);
    }
    RC access(const Key &key) override;

    std::string get_name() override;

   private:
    HillSubReplacer replacer_r_;
    HillSubReplacer replacer_s_;
    double lambda_;            // 学习率
    int32_t update_interval_;  // 更新间隔
    int32_t stable_count_{};
    double init_half_;
    double simulator_ratio_;
    double delta_bound_;
    std::list<int32_t> hit_recorder;
    int32_t recorder_hit_count{};
};

#endif  // HILL_CACHE_HILL_REPLACER_H
