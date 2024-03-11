//
// Created by Clouds on 2023/9/20.
//

#ifndef HILL_CACHE_LIRS_REPLACER_H
#define HILL_CACHE_LIRS_REPLACER_H

#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <list>
#include <map>
#include <unordered_map>

#include "replacer.h"

class LIRSReplacer : public Replacer {
   public:
    struct lirs_node;
    typedef std::list<lirs_node *>::iterator lirs_iterator;
    struct lirs_node {
        long long key;
        long long value;

        lirs_type type;
        lirs_iterator s;
        lirs_iterator q;
        lirs_iterator hirs;
        lirs_node(long long _key, long long _value, lirs_iterator ends,
                  lirs_iterator endq)
            : key(_key), value(_value), s(ends), q(endq), type(LIR) {}
        lirs_node(long long _key, long long _value, lirs_iterator ends,
                  lirs_iterator endq, lirs_type _type)
            : key(_key), value(_value), s(ends), q(endq), type(_type) {}

        void Set(lirs_type _type) { type = _type; }
    };
    LIRSReplacer(int32_t buffer_size, int32_t stats_interval,
                 double limit_ratio = 100.0f)
        : Replacer(buffer_size, stats_interval),
          cache_size_(buffer_size),
          used_size_(0),
          limit_ratio_(limit_ratio) {
        q_size_ = std::max(1, (int)(0.01 * buffer_size));
        //        q_size_ = 1;
        s_size_ = buffer_size - q_size_;
    }

    RC access(const Key &key) override;

    std::string get_name() override;

    ~LIRSReplacer() {
        for (auto it = map_.begin(); it != map_.end(); ++it) {
            // std::cout << "key: " << it->second->key << std::endl;
            delete (it->second);
        }
        //        printf("%d %d %d %d\n", tot, c_lir, c_hir_s, c_hir_ns);
    }

    void FreeOne() {
        assert(!q_.empty());

        auto pnode = q_.back();
        q_.pop_back();
        pnode->q = q_.end();

        if (IS_VALID(pnode->value)) {
            pnode->value = INVALID;
            --used_size_;
        }

        if (pnode->s != s_.end()) {
            pnode->type = NHIR;
        } else {
            // std::cout << "Free Pnode" << std::endl;
            map_.erase(pnode->key);
            delete pnode;
        }
    }

    long long Get(long long key, long long value = 10) {
        if (map_.find(key) == map_.end()) {
            return NONEVALUE;
        }
        auto p = map_[key];

        if (p->type == LIR) {
            increase_hit_count();
            assert(p->s != s_.end());
            MoveTop(p);
            c_lir++;
        } else if (p->type == HIR && IS_VALID(p->value)) {
            increase_hit_count();
            c_hir_s++;
            assert(p->q != q_.end());
            if (p->s != s_.end()) {
                p->type = LIR;

                MoveTop(p);
                Pop(p, false);
                Bottom();

                hirs_.erase(p->hirs);
                p->hirs = hirs_.end();
            } else {
                Push(p, true);
                MoveTop(p, false);

                hirs_.push_front(p);
                p->hirs = hirs_.begin();
            }
        } else {
            increase_miss_count();
            c_hir_ns++;
            assert(p->type == NHIR);
            FreeOne();
            p->value = value;

            if (p->s != s_.end()) {
                p->type = LIR;
                MoveTop(p);
                Bottom();

                hirs_.erase(p->hirs);
                p->hirs = hirs_.end();
            } else {
                assert(p->q == q_.end());
                p->type = HIR;
                Push(p, true);
                Push(p, false);

                hirs_.push_front(p);
                p->hirs = hirs_.begin();
            }
        }
        Pruning();

        return p->value;
    }

    long long Peek(long long key) {
        long long value = NONEVALUE;
        if (map_.find(key) != map_.end()) {
            value = map_[key]->value;
        }

        //        sta_.Hit(value);
        return value;
    }

    void Pruning() {
        while (!s_.empty() && NEED_PRUNING(s_.back())) {
            s_.back()->s = s_.end();

            hirs_.erase(s_.back()->hirs);
            s_.back()->hirs = hirs_.end();

            s_.pop_back();
        }
    }

    //    void Print(bool flag = false) {
    //        if (! flag) {
    //            for (auto pit = s_.begin(); pit != s_.end(); ++pit) {
    //                auto it = *pit;
    //                std::cout << "[" << it->key << ":" << it->value << " " <<
    //                it->type << "]";
    //            }
    //            std::cout << std::endl;
    //
    //            for (auto pit = q_.begin(); pit != q_.end(); ++pit) {
    //                auto it = *pit;
    //                std::cout << "[" << it->key << ":" << it->value << " " <<
    //                it->type << "]";
    //            }
    //            std::cout << std::endl;
    //        }
    //
    //        sta_.Print();
    //        std::cout << "{" << s_.size() << ":" << q_.size() << "}"
    //                  << "{" << used_size_ << ":" << s_size_ << ":" << q_size_
    //                  << "}" << std::endl;
    //    }

   private:
    int c_lir{}, c_hir_s{}, c_hir_ns{}, tot{};
    void Bottom() {
        auto bottom = s_.back();
        if (bottom->type == LIR) {
            hirs_.push_front(bottom);
            bottom->hirs = hirs_.begin();

            bottom->type = HIR;
            if (bottom->q != q_.end()) {
                Pop(bottom, false);
            }
            Push(bottom, false);
        }
    }
    // true to S, false to Q
    void Push(lirs_node *p, bool toS) {
        if (toS) {
            s_.push_front(p);
            p->s = s_.begin();

            if (s_.size() >= limit_ratio_ * s_size_) {
                lirs_node *node = hirs_.back();
                if (node->type == NHIR) {
                    s_.erase(node->s);
                    hirs_.erase(node->hirs);
                    map_.erase(node->key);
                    delete node;
                }
            }
            //                for (auto riter = s_.rbegin(); riter != s_.rend();
            //                riter++) {
            //                    auto item = *riter;
            //                    if (item->type == NHIR) {
            //                        s_.erase(std::next(riter).base());
            //                        map_.erase(item->key);
            ////                        delete item;
            //                        break;
            //                    }
            //                }
            //            }
        } else {
            q_.push_front(p);
            p->q = q_.begin();
        }

        if (map_.find(p->key) == map_.end()) {
            map_[p->key] = p;
        }
    }

    void Pop(lirs_node *p, bool fromS) {
        if (fromS) {
            assert(p->s != s_.end());
            s_.erase(p->s);
            p->s = s_.end();
        } else {
            assert(p->q != q_.end());
            q_.erase(p->q);
            p->q = q_.end();
        }
    }

    void MoveTop(lirs_node *p, bool toS = true) {
        Pop(p, toS);
        Push(p, toS);
    }

    // front -- top  back  -- bottom
    std::list<lirs_node *> s_, q_;
    std::list<lirs_node *> hirs_;
    std::map<long long, lirs_node *> map_;

    long long cache_size_, used_size_;
    long long s_size_;
    long long q_size_;
    double limit_ratio_;
    //    statistic sta_;
};

#endif  // HILL_CACHE_LIRS_REPLACER_H
