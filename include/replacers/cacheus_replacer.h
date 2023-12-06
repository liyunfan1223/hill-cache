//
// Created by Yunfan Li on 2023/9/24.
//

#ifndef HILL_CACHE_CACHEUS_REPLACER_H
#define HILL_CACHE_CACHEUS_REPLACER_H

#include "deque_dict.hpp"
#include "heap_dict.hpp"
#include "replacer.h"

class CACHEUSReplacer : public Replacer {
   public:
    class CacheusEntry {
       public:
        CacheusEntry() = default;
        CacheusEntry(Key key1, int32_t freq1, int32_t time1,
                     int32_t is_new1 = true) {
            key = key1;
            freq = freq1;
            time = time1;
            is_new = is_new1;
        }
        Key key;
        int32_t freq;
        int32_t time;
        int32_t evicted_time;
        bool is_demoted = false;
        bool is_new = true;
        bool operator<(const CacheusEntry &rhs) const {
            if (freq == rhs.freq) {
                return time > rhs.time;
            }
            return freq < rhs.time;
        }
    };
    class CacheusLearningRate {
       public:
        CacheusLearningRate(int32_t period_length) {
            learning_rate = sqrt((2.0 * log(2) / period_length));
            learning_rate_reset = std::min(std::max(learning_rate, 0.001), 1.0);
            learning_rate_curr = learning_rate;
            learning_rate_prev = 0.0;

            period_len = period_length;

            hitrate = 0;
            hitrate_prev = 0.0;
            hitrate_diff_prev = 0.0;
            hitrate_zero_count = 0;
            hitrate_nega_count = 0;
        }
        double Get() { return learning_rate; }
        double Round(double x, int digit) {
            int base = 1;
            for (int i = 1; i <= digit; i++) {
                base *= 10;
            }
            return ((int)(x * base)) / (double)base;
        }
        double updateInDeltaDirection(double learning_rate_diff,
                                      double hitrate_diff) {
            double delta = learning_rate_diff * hitrate_diff;
            delta = delta != 0 ? int(delta / fabs(delta)) : 0;
            return delta;
        }
        void updateInRandomDirection() {
            if (learning_rate >= 1) {
                learning_rate = 0.9;
            } else if (learning_rate <= 0.001) {
                learning_rate = 0.005;
            } else if (rand() % 2 == 0) {
                learning_rate = std::min(learning_rate * 1.25, 1.0);
            } else {
                learning_rate = std::max(learning_rate * 0.75, 0.001);
            }
        }
        void update(int32_t time) {
            if (time % period_len == 0) {
                double hitrate_curr = Round(hitrate / period_len, 3);
                double hitrate_diff = Round(hitrate_curr - hitrate_prev, 3);

                double delta_LR =
                    Round(learning_rate_curr, 3) - Round(learning_rate_prev, 3);
                double delta = updateInDeltaDirection(delta_LR, hitrate_diff);

                if (delta > 0) {
                    learning_rate = std::min(
                        learning_rate + fabs(learning_rate * delta_LR), 1.0);
                    hitrate_nega_count = 0;
                    hitrate_zero_count = 0;
                } else if (delta < 0) {
                    learning_rate = std::max(
                        learning_rate - fabs(learning_rate * delta_LR), 0.001);
                    hitrate_nega_count = 0;
                    hitrate_zero_count = 0;
                } else if (delta == 0 && hitrate_diff <= 0) {
                    if (hitrate_curr <= 0 && hitrate_diff == 0) {
                        hitrate_zero_count += 1;
                    }
                    if (hitrate_diff < 0) {
                        hitrate_nega_count += 1;
                        hitrate_zero_count += 1;
                    }
                    if (hitrate_zero_count >= 10) {
                        learning_rate = learning_rate_reset;
                        hitrate_zero_count = 0;
                    } else if (hitrate_diff < 0) {
                        if (hitrate_nega_count >= 10) {
                            learning_rate = learning_rate_reset;
                            hitrate_nega_count = 0;
                        } else {
                            updateInRandomDirection();
                        }
                    }
                }
                learning_rate_prev = learning_rate_curr;
                learning_rate_curr = learning_rate;
                hitrate_prev = hitrate_curr;
                hitrate_diff_prev = hitrate_diff;
                hitrate = 0;
            }
        }
        double learning_rate, learning_rate_reset, learning_rate_curr,
            learning_rate_prev, hitrate{};
        std::vector<double> learning_rates;
        int32_t period_len;
        int32_t hitrate_prev{}, hitrate_zero_count{}, hitrate_nega_count{};
        double hitrate_diff_prev{};
    };
    typedef DequeDict<CacheusEntry> CEDequeDict;
    typedef HeapDict<CacheusEntry> CEHeapDict;
    CACHEUSReplacer(int32_t buffer_size, int32_t stats_interval)
        : Replacer(buffer_size, stats_interval), lr(buffer_size) {
        history_size = buffer_size / 2;
        W[0] = W[1] = 0.5;
        double hirsRatio = 0.01;
        q_limit = std::max(1, (int)((hirsRatio * buffer_size) + 0.5));
        s_limit = buffer_size - q_limit;
        q_size = s_size = dem_count = nor_count = 0;
    }

    RC access(const Key &key) override;

    std::string get_name() override;

   private:
    void HitInS(Key key);
    void HitInQ(Key key);
    void AdjustSize(bool hitInQ);
    Key HitInLRUHist(Key key);
    Key HitInLFUHist(Key key);
    Key Evict();
    Key Miss(Key key);
    void AdjustWeights(double rewardLRU, double rewardLFU) {
        rewardLRU = exp(rewardLRU * lr.Get());
        rewardLFU = exp(rewardLFU * lr.Get());
        W[0] *= rewardLRU;
        W[1] *= rewardLFU;
        double sum = W[0] + W[1];
        W[0] /= sum;
        W[1] /= sum;
        if (W[0] >= 0.99) {
            W[0] = 0.99;
            W[1] = 0.01;
        } else if (W[1] >= 0.99) {
            W[0] = 0.01;
            W[1] = 0.99;
        }
    }
    void AddToS(Key key, int32_t freq, bool isNew = true) {
        CacheusEntry x(key, freq, ts, isNew);
        s.Set(key, x);
        lfu.Set(key, x);
        s_size += 1;
    }
    void AddToQ(Key key, int32_t freq, bool isNew = true) {
        CacheusEntry x(key, freq, ts, isNew);
        q.Set(key, x);
        lfu.Set(key, x);
        q_size += 1;
    }
    void LimitStack() {
        while (s_size >= s_limit) {
            CacheusEntry demoted = s.PopFirst();
            s_size -= 1;
            demoted.is_demoted = true;
            dem_count += 1;
            q.Set(demoted.key, demoted);
            q_size += 1;
        }
    }
    int32_t GetChoice() { return (rand() % 10000 / 10000.0) < W[0] ? 0 : 1; }
    void AddToHistory(CacheusEntry evicted, int32_t policy);
    CEDequeDict s, q, lru_hist, lfu_hist;
    CEHeapDict lfu;
    //    int32_t ts{};
    int32_t dem_count{}, s_size{}, q_size{}, s_limit{}, nor_count{}, q_limit{},
        history_size{};
    double W[2];
    CacheusLearningRate lr;
};

#endif  // HILL_CACHE_CACHEUS_REPLACER_H
