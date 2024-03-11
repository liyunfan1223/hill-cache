//
// Created by Yunfan Li on 2023/5/23.
//

#include "hill_replacer.h"

#include <iomanip>

RC HillReplacer::access(const Key &key) {
    ts++;
    if (replacer_r_.Access(key)) {
        increase_hit_count();
        hit_recorder.push_back(1);
        recorder_hit_count++;
    } else {
        increase_miss_count();
        hit_recorder.push_back(0);
    }

    if (hit_recorder.size() >= 500000) {
        if (hit_recorder.front() == 1) {
            recorder_hit_count--;
        }
        hit_recorder.pop_front();
    }

    replacer_s_.Access(key);
    if (ts % update_interval_ == 0) {
        int32_t r_mc, r_hc;
        int32_t s_mc, s_hc;
        replacer_r_.ReportAndClear(r_mc, r_hc);
        replacer_s_.ReportAndClear(s_mc, s_hc);
        double r_hr = (double)r_hc / (r_mc + r_hc);
        double s_hr = (double)s_hc / (s_mc + s_hc);
        double cur_half = replacer_r_.GetCurHalf();
        if (r_hr != 0 && s_hr != 0) {
            if (fabs(s_hr - r_hr) >= EPSILON) {
                stable_count_ = 0;
                if (s_hr > r_hr) {
                    double delta_ratio = (s_hr / r_hr - 1);
                    if (delta_ratio > delta_bound_) {
                        delta_ratio = delta_bound_;
                    }
                    replacer_r_.UpdateHalf(cur_half /
                                           (1 + delta_ratio * lambda_));
                } else {
                    double delta_ratio = (r_hr / s_hr - 1);
                    if (delta_ratio > delta_bound_) {
                        delta_ratio = delta_bound_;
                    }
                    replacer_r_.UpdateHalf(cur_half *
                                           (1 + delta_ratio * lambda_));
                }
            } else {
                stable_count_++;
                if (stable_count_ == 5) {
                    double delta_ratio = 0.1;
                    if (delta_ratio > delta_bound_) {
                        delta_ratio = delta_bound_;
                    }
                    if (cur_half < init_half_) {
                        replacer_r_.UpdateHalf(cur_half *
                                               (1 + delta_ratio * lambda_));
                    } else {
                        replacer_r_.UpdateHalf(cur_half /
                                               (1 + delta_ratio * lambda_));
                    }
                    stable_count_ = 0;
                }
            }
        }
    }
    replacer_s_.UpdateHalf(replacer_r_.GetCurHalf() * simulator_ratio_);
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
        std::cout << " Current real replacer R: " << std::setprecision(4)
                  << replacer_r_.GetCurHalf()
                  << " Interval hit rate: " << std::setprecision(4)
                  << (double)recorder_hit_count / hit_recorder.size() << '\n'
                  << replacer_r_.real_lru_[0].size() << " "
                  << replacer_r_.real_lru_[1].size() << " "
                  << replacer_r_.real_map_.size() << '\n';
    }

    return RC::SUCCESS;
}

std::string HillReplacer::get_name() { return {"Hill-Cache"}; }
