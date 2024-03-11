//
// Created by Yunfan Li on 2024/2/18.
//

#ifndef HILL_CACHE_S3FIFO_REPLACER_H
#define HILL_CACHE_S3FIFO_REPLACER_H

#include <list>

#include "def.h"
#include "replacer.h"

class S3FIFOReplacer : public Replacer {
   public:
    struct s3fifo_node {
        Key key;
        int32_t freq = 0;
    };

    S3FIFOReplacer(int32_t buffer_size, int32_t stats_interval)
        : Replacer(buffer_size, stats_interval) {
        target_len_s_ = buffer_size / 10;
        target_len_m_ = buffer_size - target_len_s_;
    }

    ~S3FIFOReplacer() {}
    RC access(const Key& key) override;
    std::string get_name() override;

   private:
    void InsertM(s3fifo_node* node) {
        node->freq = 0;
        M_.push_front(node);
    }
    void InsertS(s3fifo_node* node) { S_.push_front(node); }
    void InsertG(s3fifo_node* newNode) {
        if (G_.size() == target_len_m_) {
            s3fifo_node* tailNode = G_.back();
            G_.pop_back();
            if (tailNode->freq < 0) {
                table_.erase(tailNode->key);
                delete tailNode;
            }
        }

        newNode->freq = -1;
        G_.push_front(newNode);
    }
    void EnsureFree() {
        while (S_.size() + M_.size() >= buffer_size_) {
            if (M_.size() >= target_len_m_ || S_.size() == 0) {
                EvictM();
            } else {
                EvictS();
            }
        }
    }
    void EvictM() {
        while (M_.size() > 0) {
            s3fifo_node* tailNode = M_.back();
            M_.pop_back();
            if (tailNode->freq > 0) {
                tailNode->freq -= 1;
                M_.push_front(tailNode);
            } else {
                table_.erase(tailNode->key);
                delete tailNode;
                return;
            }
        }
        assert(false);
    }
    void EvictS() {
        while (S_.size() > 0) {
            s3fifo_node* tailNode = S_.back();
            S_.pop_back();
            if (tailNode->freq > 0) {
                InsertM(tailNode);
            } else {
                InsertG(tailNode);
            }
        }
    }
    int32_t target_len_s_;
    int32_t target_len_m_;
    std::list<s3fifo_node*> S_, M_, G_;
    std::unordered_map<Key, s3fifo_node*> table_;
};

#endif  // HILL_CACHE_S3FIFO_REPLACER_H
