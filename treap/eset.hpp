#ifndef ESET_HPP
#define ESET_HPP

#ifdef DEBUG
#include <iostream>
#endif

#include <stdexcept>
#include <random>
#include <vector>

template <typename Key, typename Compare = std::less<Key>>
class ESet {
private:
    struct Node {
        const Key *key;
        size_t s[2], rank, size;

        Node(const Key *key, size_t rank) : key(key), s{0, 0}, rank(rank), size(0) {}

        void link(size_t x, size_t d) {
            s[d] = x;
        }
    };

    class MemoryPool {
    private:
        std::vector<Node> pool;
        std::vector<const Key*> value;
        size_t ref_count;
        std::mt19937_64 gen;

    public:
        MemoryPool() : ref_count(1), gen(), pool{Node(nullptr, 0)} {}

        size_t generateNew(const Key *key) {
            value.emplace_back(key);
            pool.emplace_back(key, gen());
            pool.back().size = 1;
            return pool.size()-1;
        }

        size_t copy(size_t other) {
            pool.emplace_back(pool[other]);
            return pool.size()-1;
        }

        Node& get(size_t index) {
            return pool[index];
        }

        void update(size_t index) {
            pool[index].size = pool[pool[index].s[0]].size + pool[pool[index].s[1]].size + 1;
        }

        void link(size_t x, size_t y, size_t d) {
            pool[x].link(y, d);
            update(x);
        }

        MemoryPool* assign() {
            ref_count++;
            return this;
        }

        bool release() {
            ref_count--;
            return !ref_count;
        }

        ~MemoryPool() {
            for (auto &i : value) {
                delete i;
            }
        }

#ifdef DEBUG
        void debug_print() const {
            for (size_t i = 1; i < pool.size(); i++) {
                std::cerr << i << ": " << *pool[i].key << " " << pool[i].s[0] << " " << pool[i].s[1] << " " << pool[i].rank << " " << pool[i].size << std::endl;
            }
            std::cerr << std::endl;
        }
#endif
    };

    mutable size_t root;
    mutable MemoryPool *p;
    Compare cmp;

    std::pair<size_t, size_t> splitBelow(size_t r, const Key &key) {
        if (!r) return std::make_pair(0, 0);
        Node n = p->get(r);
        size_t x, y;
        if (cmp(*n.key, key)) {
            x = p->copy(r);
            auto pair = splitBelow(n.s[1], key);
            p->link(x, pair.first, 1);
            y = pair.second;
        } else {
            y = p->copy(r);
            auto pair = splitBelow(n.s[0], key);
            p->link(y, pair.second, 0);
            x = pair.first;
        }
        return std::make_pair(x, y);
    }

    std::pair<size_t, size_t> splitAbove(size_t r, const Key &key) {
        if (!r) return std::make_pair(0, 0);
        Node n = p->get(r);
        size_t x, y;
        if (cmp(key, *n.key)) {
            y = p->copy(r);
            auto pair = splitAbove(n.s[0], key);
            p->link(y, pair.second, 0);
            x = pair.first;
        } else {
            x = p->copy(r);
            auto pair = splitAbove(n.s[1], key);
            p->link(x, pair.first, 1);
            y = pair.second;
        }
        return std::make_pair(x, y);
    }

    size_t merge(size_t x, size_t y) {
        if (!x) return y;
        if (!y) return x;
        Node &u = p->get(x), &v = p->get(y);
        if (u.rank >= v.rank) {
            p->link(x, merge(u.s[1], y), 1);
            return x;
        } else {
            p->link(y, merge(x, v.s[0]), 0);
            return y;
        }
        return 0;
    }

    size_t nfind(const Key &key) const {
        size_t x;
        for (x=root; x;) {
            Node &n = p->get(x);
            if (cmp(*n.key, key)) {
                x = n.s[1];
            } else if (cmp(key, *n.key)) {
                x = n.s[0];
            } else return x;
        }
        return 0;
    }

    size_t findAbove(const Key &key) const {
        size_t y = 0;
        for (size_t x = root; x; ) {
            const Node &n = p->get(x);
            if (cmp(key, *n.key)) {
                y = x;
                x = n.s[0];
            } else x = n.s[1];
        }
        return y;
    }

    size_t findBelow(const Key &key) const {
        size_t y = 0;
        for (size_t x = root; x; ) {
            const Node &n = p->get(x);
            if (cmp(*n.key, key)) {
                y = x;
                x = n.s[1];
            } else x = n.s[0];
        }
        return y;
    }

    size_t findFirst() const {
        size_t x, y;
        for (x=root; x && (y=p->get(x).s[0]); x = y);
        return x;
    }

    size_t findLast() const {
        size_t x, y;
        for (x=root; x && (y=p->get(x).s[1]); x = y);
        return x;
    }

    /*
    Sum the number of elements in the set that are no more than key.
    [k <= key]
    */
    size_t count_lower(const Key &key) const {
        size_t cnt=0, x;
        for (x=root; x; ) {
            Node &n = p->get(x);
            if (!cmp(key, *n.key)) {
                cnt += p->get(n.s[0]).size + 1;
                x = n.s[1];
            } else {
                x = n.s[0];
            }
        }
        return cnt;
    }

    /*
    Sum the number of elements in the set that are strictly less than key.
    [k < key]
    */
    size_t count_upper(const Key &key) const {
        size_t cnt=0, x;
        for (x=root; x; ) {
            Node &n = p->get(x);
            if (cmp(*n.key, key)) {
                cnt += p->get(n.s[0]).size + 1;
                x = n.s[1];
            } else {
                x = n.s[0];
            }
        }
        return cnt;
    }

    public:

    class iterator {
        friend class ESet;
    private:
        const Key *key;
        const ESet *from;

        iterator(size_t ptr, const ESet *from): key(from->p->get(ptr).key), from(from) {}

    public:
        iterator(): key(0), from(nullptr) {}

        const Key& operator*() const {
            return *key;
        }

        const Key* operator->() const {
            return key;
        }

        iterator& operator++() {
            if (key) key = from->p->get(from->findAbove(*key)).key;
            return *this;
        }

        iterator& operator--() {
            const Key *tmp;
            if (key) tmp = from->p->get(from->findBelow(*key)).key;
            else tmp = from->p->get(from->findLast()).key;
            if (tmp) key = tmp;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        bool operator==(const iterator& other) const {    
            return key == other.key && from == other.from;
        }

        bool operator!=(const iterator& other) const {
            return key != other.key || from!= other.from;
        }
    };

    ESet(): root(0), p(new MemoryPool()) {}

    ESet(const ESet& other) {
        root = other.root;
        p = other.p->assign();
    }

    ESet& operator=(const ESet& other) {
        if (&other == this) return *this;
        if (p && p->release()) delete p;
        root = other.root;
        p = other.p->assign();
        return *this;
    }

    ESet(ESet&& other): root(other.root), p(other.p) {
        other.p = nullptr;
    }

    ESet& operator=(ESet&& other) {
        if (&other == this) return *this;
        if (p && p->release()) delete p;
        root = other.root;
        p = other.p;
        other.p = nullptr;
        return *this;
    }

    ~ESet() {
        if (p && p->release()) delete p;
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        Key key(std::forward<Args>(args)...);
        if (!root) {
            root = p->generateNew(new Key(std::move(key)));
            return std::make_pair(iterator(root, this), true);
        }

        size_t x, y, z;
        if (x = nfind(key)) return std::make_pair(iterator(x, this), false);

        auto pair = splitAbove(root, key);
        x = pair.first, z = pair.second;
        y = p->generateNew(new Key(std::move(key)));
        root = merge(merge(x, y), z);

        return std::make_pair(iterator(y, this), true);
    }

    size_t erase(const Key& key) {
        size_t x, y, z;
        if (!(x = nfind(key))) return 0;

        auto pair = splitBelow(root, key);
        x = pair.first, y = pair.second;
        pair = splitAbove(y, key);
        y = pair.first, z = pair.second;
        root = merge(x, z);
        return 1;
    }

    size_t size() const {
        return p->get(root).size;
    }

    size_t range(const Key &l, const Key &r) const {
        return count_lower(r) - count_upper(l);
    }

    iterator find(const Key& key) const {
        return iterator(nfind(key), this);
    }

    iterator lower_bound(const Key &key) const {
        size_t x, y=0;
        for (x=root; x; ) {
            Node &n = p->get(x);
            if (!cmp(*n.key, key)) {
                y = x;
                x = n.s[0];
            } else x = n.s[1];
        }
        return iterator(y, this);
    }

    iterator upper_bound(const Key &key) const {
        size_t x, y=0;
        for (x=root; x; ) {
            Node &n = p->get(x);
            if (cmp(key, *n.key)) {
                y = x;
                x = n.s[0];
            } else x = n.s[1];
        }
        return iterator(y, this);
    }

    iterator begin() const {
        return iterator(findFirst(), this);
    }

    iterator end() const {
        return iterator(0, this);
    }

#ifdef DEBUG
    void debug_print() const {
        std::cerr << "root: " << root << std::endl;
        p->debug_print();
    }
#endif
};

#endif