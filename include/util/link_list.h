//
// Created by Yunfan Li on 2022/9/28.
//

#pragma once

#include <def.h>

template <typename T>
struct LinkNode;

template <typename T>
class LinkList;

template <typename T>
struct LinkNode {
    LinkNode() {}
    LinkNode(T key) : key(key) {}
    LinkNode *pred = nullptr;
    LinkNode *next = nullptr;
    T key;
};

template <typename T>
class LinkList {
   public:
    LinkList() {}
    ~LinkList();
    RC push_front(T key);
    RC push_back(T key);
    RC pop_back();
    RC pop_front();
    RC remove(LinkNode<T> *node);

    LinkNode<T> *head = nullptr;
    LinkNode<T> *tail = nullptr;
    int32_t size = 0;
};
