//
// Created by Yunfan Li on 2022/9/28.
//

#include "link_list.h"

template <typename T>
LinkList<T>::~LinkList() {
    while (head != nullptr) {
        auto old = head;
        head = head->next;
        delete old;
    }
}

template <typename T>
RC LinkList<T>::push_front(T key) {
    LinkNode<T> *new_node = new LinkNode<T>(key);
    if (head != nullptr) {
        head->pred = new_node;
    }
    new_node->next = head;
    if (tail == nullptr) {
        tail = new_node;
    }
    head = new_node;
    size++;
    return RC::SUCCESS;
}

template <typename T>
RC LinkList<T>::push_back(T key) {
    LinkNode<T> *new_node = new LinkNode<T>(key);
    if (tail != nullptr) {
        tail->next = new_node;
    }
    new_node->pred = tail;
    if (head == nullptr) {
        head = new_node;
    }
    tail = new_node;
    size++;
    return RC::SUCCESS;
}

template <typename T>
RC LinkList<T>::pop_back() {
    return remove(tail);
}

template <typename T>
RC LinkList<T>::pop_front() {
    return remove(head);
}

template <typename T>
RC LinkList<T>::remove(LinkNode<T> *node) {
    if (head == node) {
        head = node->next;
    }
    if (tail == node) {
        tail = node->pred;
    }
    if (node->pred != nullptr) {
        node->pred->next = node->next;
    }
    if (node->next != nullptr) {
        node->next->pred = node->pred;
    }
    size--;
    delete node;
    return RC::SUCCESS;
}

template class LinkList<Key>;
