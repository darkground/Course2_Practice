#ifndef PRIOQUEUE_H
#define PRIOQUEUE_H

#include <queue>

template<typename T, typename priority_t>
struct PriorityQueue {
    typedef std::pair<priority_t, T> element;
    std::priority_queue<element, std::vector<element>, std::greater<element>> elements;

    inline bool empty() const {
        return elements.empty();
    }

    inline void put(T item, priority_t priority) {
        elements.emplace(priority, item);
    }

    T get() {
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};

#endif // PRIOQUEUE_H
