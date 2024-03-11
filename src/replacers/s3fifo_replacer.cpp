//
// Created by Yunfan Li on 2024/2/18.
//

#include "s3fifo_replacer.h"

RC S3FIFOReplacer::access(const Key &key) {
    ts++;
    if (table_.find(key) != table_.end()) {
        // increase_hit_count();
        s3fifo_node *node = table_.at(key);
        if (node->freq < 0) {
            increase_miss_count();
            node->freq = 0;
            EnsureFree();
            InsertM(node);
        } else {
            increase_hit_count();
            int freq = std::min(node->freq + 1, 3);
            node->freq = freq;
        }
    } else {
        increase_miss_count();
        s3fifo_node *newNode = new s3fifo_node();
        newNode->key = key;
        table_[key] = newNode;

        EnsureFree();
        InsertS(newNode);
    }
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string S3FIFOReplacer::get_name() { return {"S3FIFO"}; }