//
// Created by Yunfan Li on 2022/10/5.
//

#include "lru_list.h"

template <typename T>
RC LRUList<T>::push_front(Key key) {
    if (u_map.count(key) != 0) {
        remove(key);
    }
    LinkList<T>::push_front(key);
    u_map[key] = LinkList<T>::head;
    return RC::SUCCESS;
}

template <typename T>
RC LRUList<T>::pop_back() {
    LinkNode<T> *old_tail = LinkList<T>::tail;
    Key key = old_tail->key;
    u_map.erase(key);
    LinkList<T>::pop_back();
    return RC::SUCCESS;
}

template <typename T>
RC LRUList<T>::pop_back(Key &key) {
    LinkNode<T> *old_tail = LinkList<T>::tail;
    key = old_tail->key;
    u_map.erase(key);
    LinkList<T>::pop_back();
    return RC::SUCCESS;
}

template <typename T>
RC LRUList<T>::remove(const Key &key) {
    LinkList<T>::remove(u_map.at(key));
    u_map.erase(key);
    return RC::SUCCESS;
}

template <typename T>
int32_t LRUList<T>::count(const Key &key) {
    return u_map.count(key);
}

template <typename T>
int32_t LRUList<T>::size() {
    return LinkList<T>::size;
}

template class LRUList<Key>;