//
// Created by Yunfan Li on 2023/9/24.
//

#include "cacheus_replacer.h"

RC CACHEUSReplacer::access(const Key &key) {
    bool miss = false;
    Key evicted = -1;
    ts += 1;
    lr.update(ts);
    if (s.Contain(key)) {
        HitInS(key);
    } else if (q.Contain(key)) {
        HitInQ(key);
    } else if (lru_hist.Contain(key)) {
        miss = true;
        evicted = HitInLRUHist(key);
    } else if (lfu_hist.Contain(key)) {
        miss = true;
        evicted = HitInLFUHist(key);
    } else {
        miss = true;
        evicted = Miss(key);
    }
    if (miss) {
        increase_miss_count();
    } else {
        increase_hit_count();
    }
    if (enable_interval_stats_ && ts % stats_interval == 0) {
        std::cout << stats();
    }
    return RC::SUCCESS;
}

std::string CACHEUSReplacer::get_name() { return {"CACHEUS"}; }

void CACHEUSReplacer::HitInS(Key key) {
    CacheusEntry entry = s.Get(key);
    entry.time = ts;
    s.Set(key, entry);
    entry.freq += 1;
    lfu.Set(key, entry);
}

void CACHEUSReplacer::HitInQ(Key key) {
    CacheusEntry entry = q.Get(key);
    entry.time = ts;
    entry.freq += 1;
    lfu.Update(key, entry);
    if (entry.is_demoted) {
        AdjustSize(true);
        entry.is_demoted = false;
        dem_count -= 1;
    }
    q.Remove(key);
    q_size -= 1;

    if (s_size >= s_limit) {
        CacheusEntry entry_y = s.PopFirst();
        entry_y.is_demoted = true;
        dem_count += 1;
        s_size -= 1;
        q.Set(entry_y.key, entry_y);
        q_size += 1;
    }
    s.Set(key, entry);
    s_size += 1;
}

void CACHEUSReplacer::AdjustSize(bool hitInQ) {
    if (hitInQ) {
        dem_count = std::max(1, dem_count);
        s_limit = std::min(
            buffer_size_ - 1,
            s_limit + std::max(1, (int)(((float)nor_count / dem_count) + 0.5)));
        q_limit = buffer_size_ - s_limit;
    } else {
        nor_count = std::max(1, nor_count);
        q_limit = std::min(
            buffer_size_ - 1,
            q_limit + std::max(1, (int)(((float)dem_count / nor_count) + 0.5)));
        s_limit = buffer_size_ - q_limit;
    }
}

Key CACHEUSReplacer::Evict() {
    CacheusEntry evicted_entry;
    CacheusEntry lru_entry = q.GetFirst();
    CacheusEntry lfu_entry = lfu.GetMin();

    evicted_entry = lru_entry;
    int32_t policy = GetChoice();

    if (lru_entry.key == lfu_entry.key) {
        policy = -1;
        evicted_entry = lru_entry;
    } else if (policy == 0) {
        evicted_entry = lru_entry;
        q.Remove(evicted_entry.key);
        q_size -= 1;
    } else if (policy == 1) {
        evicted_entry = lfu_entry;
        if (s.Contain(evicted_entry.key)) {
            s.Remove(evicted_entry.key);
            s_size -= 1;
        } else if (q.Contain(evicted_entry.key)) {
            q.Remove(evicted_entry.key);
            q_size -= 1;
        }
    }
    if (evicted_entry.is_demoted) {
        dem_count -= 1;
        evicted_entry.is_demoted = false;
    }
    if (policy == -1) {
        q.Remove(evicted_entry.key);
        q_size -= 1;
    }

    lfu.Remove(evicted_entry.key);
    evicted_entry.evicted_time = ts;

    AddToHistory(evicted_entry, policy);
    return evicted_entry.key;
}

void CACHEUSReplacer::AddToHistory(CACHEUSReplacer::CacheusEntry x,
                                   int32_t policy) {
    CEDequeDict *policy_history = nullptr;
    if (policy == 0) {
        if (x.is_new) {
            nor_count += 1;
        }
        policy_history = &lru_hist;
    } else if (policy == 1) {
        policy_history = &lfu_hist;
    } else if (policy == -1) {
        return;
    }

    if (policy_history->Size() == history_size) {
        CacheusEntry evicted = policy_history->GetFirst();
        policy_history->Remove(evicted.key);
        if (policy_history == &lru_hist && evicted.is_new) {
            nor_count -= 1;
        }
    }
    policy_history->Set(x.key, x);
}

Key CACHEUSReplacer::HitInLRUHist(Key key) {
    Key evicted = -1;

    CacheusEntry entry = lru_hist.Get(key);
    lru_hist.Remove(key);
    if (entry.is_new) {
        nor_count -= 1;
        entry.is_new = false;
        AdjustSize(false);
    }
    AdjustWeights(-1, 0);

    if (s_size + q_size >= buffer_size_) {
        evicted = Evict();
    }

    AddToS(entry.key, entry.freq, false);
    LimitStack();
    return evicted;
}

Key CACHEUSReplacer::HitInLFUHist(Key key) {
    Key evicted = -1;

    CacheusEntry entry = lfu_hist.Get(key);
    lfu_hist.Remove(key);
    AdjustWeights(0, -1);

    if (s_size + q_size >= buffer_size_) {
        evicted = Evict();
    }

    AddToS(entry.key, entry.freq, false);
    LimitStack();
    return evicted;
}

Key CACHEUSReplacer::Miss(Key key) {
    Key evicted;
    int32_t freq = 1;
    if (s_size < s_limit && q_size == 0) {
        AddToS(key, freq, false);
    } else if (s_size + q_size < buffer_size_ && q_size < q_limit) {
        AddToQ(key, freq, false);
    } else {
        if (s_size + q_size >= buffer_size_) {
            evicted = Evict();
        }

        AddToQ(key, freq, true);
        LimitStack();
    }
    return evicted;
}
