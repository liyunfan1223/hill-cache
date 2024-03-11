//
// Created by l50029536 on 2022/9/29.
//

#pragma once

#include <set>
#include <unordered_map>

#include "replacer.h"

class LFUReplacer : public Replacer {
   public:
    struct Status {
        Status(int32_t freq, int32_t timestamp, Key key)
            : freq(freq), timestamp(timestamp), key(key) {}
        int32_t freq;
        int32_t timestamp;
        Key key;
        bool operator<(const Status &rhs) const {
            if (freq != rhs.freq) {
                return freq < rhs.freq;
            } else if (timestamp != rhs.timestamp)
                return timestamp < rhs.timestamp;
            return key < rhs.key;
        }
    };
    LFUReplacer(int32_t buffer_size, int32_t stats_interval)
        : Replacer(buffer_size, stats_interval) {}

    ~LFUReplacer() {}
    RC access(const Key &key) override;

    std::string get_name() override;

   private:
    std::set<Status> buffer_set_;
    std::unordered_map<Key, std::set<Status>::iterator> u_map_;
};
