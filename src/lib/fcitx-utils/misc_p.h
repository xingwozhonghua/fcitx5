/*
 * SPDX-FileCopyrightText: 2016-2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX_UTILS_MISC_P_H_
#define _FCITX_UTILS_MISC_P_H_

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <config.h>

namespace fcitx {

template <typename M, typename K>
decltype(&std::declval<M>().begin()->second) findValue(M &&m, K &&key) {
    auto iter = m.find(key);
    if (iter != m.end()) {
        return &iter->second;
    }
    return nullptr;
}

template <typename T>
class OrderedSet {
    typedef std::list<T> OrderList;

public:
    auto begin() { return order_.begin(); }

    auto end() { return order_.end(); }
    auto begin() const { return order_.begin(); }

    auto end() const { return order_.end(); }

    bool empty() const { return order_.empty(); }

    const T &front() const { return order_.front(); }

    bool pushFront(const T &v) {
        if (dict_.count(v)) {
            return false;
        }
        order_.emplace_front(v);
        dict_.insert(std::make_pair(v, order_.begin()));
        return true;
    }

    bool pushBack(const T &v) {
        if (dict_.count(v)) {
            return false;
        }
        order_.emplace_back(v);
        dict_.insert(std::make_pair(v, std::prev(order_.end())));
        return true;
    }

    void moveToTop(const T &v) {
        auto iter = dict_.find(v);
        if (iter == dict_.end()) {
            return;
        }
        if (iter->second != order_.begin()) {
            order_.splice(order_.begin(), order_, iter->second);
        }
    }

    auto size() const { return order_.size(); }

    void pop() {
        dict_.erase(order_.back());
        order_.pop_back();
    }

    bool insert(const T &before, const T &v) {
        if (dict_.count(v)) {
            return false;
        }
        typename OrderList::iterator iter;
        if (auto dictIter = dict_.find(before); dictIter != dict_.end()) {
            iter = dictIter->second;
        } else {
            iter = order_.end();
        }
        auto newIter = order_.insert(iter, v);
        dict_.insert(std::make_pair(v, newIter));
        return true;
    }

    bool contains(const T &v) const { return !!dict_.count(v); }

    bool remove(const T &v) {
        auto iter = dict_.find(v);
        if (iter == dict_.end()) {
            return false;
        }
        order_.erase(iter->second);
        dict_.erase(iter);
        return true;
    }

    const OrderList &order() const { return order_; }

private:
    std::unordered_map<T, typename OrderList::iterator> dict_;
    OrderList order_;
};

static inline int safePipe(int pipefd[2]) {
#ifdef HAVE_PIPE2
    return ::pipe2(pipefd, O_NONBLOCK | O_CLOEXEC);
#else
    int ret = ::pipe(pipefd);
    if (ret == -1)
        return -1;
    ::fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
    ::fcntl(pipefd[0], F_SETFL, ::fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(pipefd[1], F_SETFL, ::fcntl(pipefd[1], F_GETFL) | O_NONBLOCK);
    return 0;
#endif
}

} // namespace fcitx

#endif // _FCITX_UTILS_MISC_P_H_
