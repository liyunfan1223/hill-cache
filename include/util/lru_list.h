//
// Created by Yunfan Li on 2022/10/5.
//

#pragma once

#include <unordered_map>

#include "link_list.h"

template <typename T>
class LRUList : public LinkList<T> {
   public:
    RC push_front(Key key);
    RC pop_back();
    RC pop_back(Key &key);
    RC remove(const Key &key);
    int32_t count(const Key &key);
    int32_t size();
    std::unordered_map<Key, LinkNode<T> *> u_map;
};