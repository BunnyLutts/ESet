#ifndef ESET_HPP

#define ESET_HPP

#include <stdexcept>

#ifdef DEBUG
#include <iostream>
#endif

// namespace splay {
    template <typename Key, typename Compare = std::less<Key>>
    class ESet {

    private:
        struct Node {
            const Key *key;
            int size;
            mutable Node *s[2], *fa;

            Node() : key(nullptr), size(0), s{nullptr, nullptr}, fa(nullptr) {}
            Node(const Key *key) : key(key), size(1), s{nullptr, nullptr}, fa(nullptr) {}

            void link(int t, Node *x) {
                s[t] = x;
                if (x) x->fa = this;
            }

            ~Node() {
                if (key) delete key;
            }
        };

        Compare cmp;
        mutable Node *root;

        void recollect(Node *x) {
            if (!x) return;
            recollect(x->s[0]);
            recollect(x->s[1]);
            delete x;
        }

        int dir(Node *x) const {
            return x->fa->s[1] == x;
        }

        size_t getSize(Node *x) const {
            return x ? x->size : 0;
        }

        void update(Node *x) const {
            x->size = getSize(x->s[0]) + getSize(x->s[1]) + 1;
        }
        
        void updateToRoot(Node *x) {
            for (; x; ) {
                update(x);
                x = x->fa;
            }
        }

        void rotate(Node *x) const {
            int t = dir(x);
            Node *y = x->fa, *z = y->fa;
            if (z) z->link(dir(y), x);
            else {
                root = x;
                x->fa = nullptr;
            }

            y->link(t, x->s[t ^ 1]);
            x->link(t^1, y);
            update(y);
            update(x);
        }

        void splay(Node *x, Node *y) const {
            for (; x->fa != y; ) {
                if (x->fa->fa != y && dir(x) == dir(x->fa)) rotate(x->fa);
                rotate(x);
            }
            if (!y) root = x;
        }

        Node* nfind(const Key &key) const {
            Node *x;
            for (x = root; x; ) {
                if (cmp(key, *x->key)) {
                    x = x->s[0];
                } else if (cmp(*x->key, key)) {
                    x = x->s[1];
                } else break;
            }
            return x;
        }

        Node* nlower_bound(const Key &key) const {
            if (!root) return nullptr;
            Node *x, *tar = nullptr;
            for (x=root; x; ) {
                if (!cmp(*x->key, key)) {
                    tar = x;
                    x = x->s[0];
                } else x = x->s[1];
            }
            return tar;
        }

        Node* nupper_bound(const Key &key) const {
            Node *x, *tar = nullptr;
            for(x=root; x; ) {
                if (cmp(key, *x->key)) {
                    tar = x;
                    x = x->s[0];
                } else x = x->s[1];
            }
            return tar;
        }

        Node* findNext(Node *x) const {
            if (!x) return nullptr;
            splay(x, nullptr);
            Node *y = x->s[1];
            for (; y && y->s[0]; y = y->s[0]);
            return y;
        }

        Node* findPrev(Node *x) const {
            if (!x) return nullptr;
            splay(x, nullptr);
            Node *y = x->s[0];
            for (; y && y->s[1]; y = y->s[1]);
            return y;
        }

        Node* findPrevOrStay(Node *x) const {
            if (!x) return nullptr;
            splay(x, nullptr);
            if (!x->s[0]) return x;
            Node *y = x->s[0];
            for (; y && y->s[1]; y = y->s[1]);
            return y;
        }

        Node* findLast() const {
            Node *x = root;
            for (; x && x->s[1]; x = x->s[1]);
            return x;
        }

        Node* clone(Node *x, const ESet &other) {
            if (!x) return nullptr;
            Node *y = new Node(new Key(*x->key));
            y->s[0] = clone(x->s[0], other);
            y->s[1] = clone(x->s[1], other);
            if (y->s[0]) y->s[0]->fa = y;
            if (y->s[1]) y->s[1]->fa = y;
            update(y);
            return y;
        }

    public:

        class iterator {
            friend class ESet<Key, Compare>;
        private:
            const ESet *from;
            Node *ptr;

            iterator(Node *ptr, const ESet *from) : ptr{ptr}, from{from} {}

        public:
            iterator() : ptr{nullptr}, from{nullptr} {}

            const Key& operator*() const { 
                if (!ptr) throw std::out_of_range("Out of range");
                return *ptr->key; 
            }

            const Key* operator->() const { 
                if (!ptr) throw std::out_of_range("Out of range");
                return ptr->key; 
            }

            iterator& operator++() {
                if (ptr) ptr = from->findNext(ptr);
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            iterator& operator--() {
                if (ptr) ptr = from->findPrevOrStay(ptr);
                else ptr = from->findLast();
                return *this;
            }

            iterator operator--(int) {
                iterator tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const iterator &other) const {
                return from == other.from && ptr == other.ptr;
            }

            bool operator!=(const iterator &other) const {
                return from != other.from || ptr != other.ptr;
            }
        };
        
        ESet() : root{nullptr}, cmp{} {}

        ESet(const ESet &other) : root{nullptr}, cmp{} {
            root = clone(other.root, other);
        }

        ESet& operator=(const ESet &other) {
            if (&other == this) return *this;
            recollect(root);
            root = clone(other.root, other);
            return *this;
        }

        ESet(ESet &&other) : root{std::move(other.root)}, cmp{} {
            other.root = nullptr;
        }

        ESet& operator=(ESet &&other) noexcept {
            if (&other == this) return *this;
            recollect(root);
            root = std::move(other.root);
            other.root = nullptr;
            return *this;
        }

        ~ESet() {
            recollect(root);
        }

        template<class... Args>
        std::pair<iterator, bool> emplace(Args&&... args) {
            Key key = Key(std::forward<Args>(args)...);
            if (!root) {
                root = new Node(new Key(std::move(key)));
                return std::make_pair(iterator(root, this), true);
            }
            Node *x=nullptr, *y, *z;
            for (Node *p = root; p; ) {
                if (!cmp(key, *p->key)) {
                    x = p;
                    p = p->s[1];
                } else {
                    if (!p->s[0] && !x) x = p;
                    p = p->s[0];
                }
            }
            splay(x, nullptr);
            if (!cmp(*x->key, key) && !cmp(key, *x->key)) return std::make_pair(iterator(x, this), false);
            else if (cmp(key, *x->key)) {
                z = new Node(new Key(std::move(key)));
                x->link(0, z);
                updateToRoot(z);
                splay(z, nullptr);
                return std::make_pair(iterator(z, this), true);
            } else {
                y = findNext(x), z = new Node(new Key(std::move(key)));
                if (y) {
                    splay(y, x);
                    y->link(0, z);
                } else {
                    x->link(1, z);
                }
                updateToRoot(z);
                splay(z, nullptr);
                return std::make_pair(iterator(z, this), true);
            }
            return std::make_pair(iterator(nullptr, this), false);
        }

        size_t erase(const Key &key) {
            Node *p = nfind(key);
            if (!p) return 0;
            splay(p, nullptr);
            if (!p->s[0]) {
                root = p->s[1];
                if (p->s[1]) p->s[1]->fa = nullptr;
                delete p;
            } else if (!p->s[1]) {
                root = p->s[0];
                p->s[0]->fa = nullptr;
                delete p;
            } else {
                Node *x = findPrev(p), *y = findNext(p);
                splay(x, nullptr);
                splay(y, x);
                y->s[0] = nullptr;
                delete p;
                updateToRoot(y);
            }
            return 1;
        }

        iterator begin() const noexcept {
            Node *x;
            for (x=root; x && x->s[0]; x = x->s[0]);
            return iterator(x, this);
        }

        iterator end() const noexcept {
            return iterator(nullptr, this);
        }

        size_t size() const noexcept {
            return getSize(root);
        }

        size_t range(const Key &l, const Key &r) const {
            if (root == nullptr) return 0;
            Node *x = nlower_bound(l);
            if (!x) return 0;
            splay(x, nullptr);
            Node *y = nupper_bound(r);
            if (y==x) return 0;
            if (!y) return getSize(x->s[1])+1;
            splay(y, x);
            return getSize(y->s[0])+1;
        }

        iterator find(const Key &key) const {
            return iterator(nfind(key), this);
        }

        iterator lower_bound(const Key &key) const {
            return iterator(nlower_bound(key), this);
        }

        iterator upper_bound(const Key &key) const {
            return iterator(nupper_bound(key), this);
        }


    #ifdef DEBUG
        void debug_print(Node *ptr, int x) const {
            if (!ptr) return;
            std::cerr << x << ": " <<  "size: " << ptr->size << ", key: " << *ptr->key << "\n";
            debug_print(ptr->s[0], x*2);
            debug_print(ptr->s[1], x*2+1);
        }

        void debug_print() const {
            debug_print(root, 1);
        }
    #endif
    };
// }

#endif