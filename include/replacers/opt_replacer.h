//
// Created by Yunfan Li on 2022/10/9.
//

#pragma once

#include "link_list.h"
#include "replacer.h"

class OPTReplacer : public Replacer {
   public:
    explicit OPTReplacer(int32_t bufferSize, int32_t stats_interval,
                         std::vector<Key> &access_list)
        : Replacer(bufferSize, stats_interval), access_list_(access_list) {
        int timeStamp = 0;
        for (auto key : access_list_) {
            key_access_u_map_[key].push_back(timeStamp++);
        }
    }
    ~OPTReplacer() {}
    RC access(const Key &key) override;

    std::string get_name() override;
    RC check_consistency() override;

   private:
    struct Status {
        Status(int32_t future_access_timestamp, Key key)
            : future_access_timestamp(future_access_timestamp), key(key) {}
        int32_t future_access_timestamp;
        Key key;
        bool operator<(const Status &rhs) const {
            return future_access_timestamp > rhs.future_access_timestamp;
        }
    };
    std::vector<Key> &access_list_;
    std::unordered_map<Key, LinkList<int32_t>> key_access_u_map_;
    std::unordered_map<Key, std::set<Status>::iterator> u_map_;
    std::set<Status> buffer_set_;
};
