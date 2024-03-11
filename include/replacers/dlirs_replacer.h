//
// Created by Clouds on 2023/9/20.
//

#ifndef HILL_CACHE_DLIRS_REPLACER_H
#define HILL_CACHE_DLIRS_REPLACER_H

#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <list>
#include <map>
#include <unordered_map>

#include "deque_dict.hpp"
#include "replacer.h"

class DLIRSReplacer : public Replacer {
   public:
    class DLIRSEntry {
       public:
        DLIRSEntry() = default;
        DLIRSEntry(Key key1, bool is_lir1, bool is_demoted1 = false,
                   bool in_cache1 = true) {
            key = key1;
            is_lir = is_lir1;
            is_demoted = false;
            in_cache = in_cache1;
        }
        Key key;
        bool is_lir;
        bool is_demoted;
        bool in_cache;
    };
    double Round(double x, int digit) {
        int base = 1;
        for (int i = 1; i <= digit; i++) {
            base *= 10;
        }
        return ((int)(x * base)) / (double)base;
    }
    typedef DequeDict<DLIRSEntry> DEDequeDict;

    DLIRSReplacer(int32_t buffer_size, int32_t stats_interval,
                  double limit_ratio = 100.0f)
        : Replacer(buffer_size, stats_interval), limit_ratio_(limit_ratio) {
        hirs_ratio = 0.01;
        hirs_limit = std::max(1, (int)(buffer_size * hirs_ratio + 0.5));
        lirs_limit = buffer_size - hirs_limit;
        hirs_count = 0;
        lirs_count = 0;
        demoted = 0;
        nonresident = 0;
        time = 0;
    }
    void hitLIR(Key key) {
        auto lru_lir = lirs.GetFirst();
        auto x = lirs.Get(key);
        lirs.Set(key, x);
        if (lru_lir.key == x.key) {
            Prune();
        }
    }
    void Prune() {
        while (lirs.Size()) {
            auto x = lirs.GetFirst();
            if (x.is_lir) {
                break;
            }

            lirs.Remove(x.key);
            hirs.Remove(x.key);

            if (!x.in_cache) {
                nonresident -= 1;
            }
        }
    }
    bool hitHIRinLIRS(Key key) {
        auto x = lirs.Get(key);
        bool in_cache = x.in_cache;

        x.is_lir = true;

        lirs.Remove(key);
        hirs.Remove(key);

        if (in_cache) {
            q.Remove(key);
            hirs_count -= 1;
        } else {
            AdjustSize(true);
            x.in_cache = true;
            nonresident -= 1;
        }

        while (lirs_count >= lirs_limit) {
            ejectLIR();
        }
        while (hirs_count + lirs_count >= buffer_size_) {
            ejectHIR();
        }

        lirs.Set(key, x);
        lirs_count += 1;
        return !in_cache;
    }
    void ejectLIR() {
        auto lru = lirs.PopFirst();
        lirs_count -= 1;
        lru.is_lir = false;
        lru.is_demoted = true;
        demoted += 1;
        q.Set(lru.key, lru);
        hirs_count += 1;
        Prune();
    }
    void ejectHIR() {
        auto lru = q.PopFirst();
        if (lirs.Contain(lru.key)) {
            lru.in_cache = false;
            nonresident += 1;
            lirs.InplaceUpdate(lru.key, lru);
            hirs.InplaceUpdate(lru.key, lru);
        }
        if (lru.is_demoted) {
            demoted -= 1;
        }

        hirs_count -= 1;
    }
    void AdjustSize(bool h) {
        if (h) {
            hirs_limit = std::min(
                buffer_size_ - 1,
                hirs_limit +
                    std::max(1, (int)((double)demoted / nonresident + 0.5)));
            lirs_limit = buffer_size_ - hirs_limit;
        } else {
            lirs_limit = std::min(
                buffer_size_ - 1,
                lirs_limit +
                    std::max(1, (int)((double)nonresident / demoted + 0.5)));
            hirs_limit = buffer_size_ - lirs_limit;
        }
    }
    void hitHIRinQ(Key key) {
        auto x = q.Get(key);
        if (x.is_demoted) {
            AdjustSize(false);
            x.is_demoted = false;
            demoted -= 1;
        }
        q.Set(key, x);
        lirs.Set(key, x);
        hirs.Set(key, x);
        LimitStack();
    }
    void LimitStack() {
        while (hirs_count + lirs_count + nonresident >
               limit_ratio_ * buffer_size_) {
            auto lru = hirs.PopFirst();
            lirs.Remove(lru.key);
            if (!lru.in_cache) {
                nonresident -= 1;
            }
        }
    }
    void Miss(Key key) {
        if (lirs_count < lirs_limit && hirs_count == 0) {
            auto x = DLIRSEntry(key, true);
            lirs.Set(key, x);
            lirs_count += 1;
            return;
        }

        while (hirs_count + lirs_count >= buffer_size_) {
            while (lirs_count > lirs_limit) {
                ejectLIR();
            }
            ejectHIR();
        }

        auto x = DLIRSEntry(key, false);
        lirs.Set(key, x);
        hirs.Set(key, x);
        q.Set(key, x);

        hirs_count += 1;
        LimitStack();
    }
    RC access(const Key &key) override;

    std::string get_name() override;
    double hirs_ratio, limit_ratio_;
    int32_t hirs_limit, lirs_limit, hirs_count, lirs_count, demoted,
        nonresident;
    DEDequeDict lirs, hirs, q;
    int32_t time;
};

#endif  // HILL_CACHE_DLIRS_REPLACER_H
