//
// Created by Yunfan Li on 2023/2/9.
//

#pragma once

#include "lazy_update_heap.h"
#include "replacer.h"

class LRFUReplacer : public Replacer {
   public:
    LRFUReplacer(int32_t buffer_size, int32_t stats_interval,
                 double lambda = 1e-4, bool ref_size = false)
        : Replacer(buffer_size, stats_interval),
          lambda_(ref_size ? 1 / (lambda * buffer_size) : lambda),
          lu_heap_(pow(0.5, lambda_)),
          lu_heap_store_(pow(0.5, lambda_)) {}

    RC access(const Key &key) override;

    std::string get_name() override;

   private:
    double lambda_;
    LUHeap lu_heap_;
    LUHeap lu_heap_store_;
};
