#ifndef ESET_H

#define ESET_H

#include <functional>

namespace sjtu {
    template <class Key, class Compare = std::less<Key> >
    class ESet {
    private:
        struct Node {
            Node *s[2], *fa;
            Key *key;
            size_t size;
            bool black;

            //Initially red
            Node() : s{nullptr, nullptr}, fa{nullptr}, key{nullptr}, size{0}, black{false} {}

            ~Node() {
                if (key) delete key;
                key = nullptr;
            }

            void link(int pos, Node *son) {
                son->fa = this;
                this->s[pos] = son;
            }
        };

        Node *root, *nil;
        Compare cmp;

        void recollect(Node *ptr) {
            if (ptr == nil) return;
            recollect(ptr->s[0]), recollect(ptr->s[1]);
            delete ptr;
        }

        Node* clone(Node *src, ESet &src_set) {
            Node *dest = new Node;
            dest->black = src->black;
            dest->size = src->size;
            dest->key = new Key(*src->key);
            dest->link(0, clone(src->s[0], src_set));
            dest->link(1, clone(src->s[1], src_set));
        }

        Node* newLeaf(Key &&key) {
            Node *leaf = new Node;
            leaf->link(0, nil);
            leaf->link(1, nil);
            leaf->black = false;
            leaf->size = 1;
            leaf->key = new Key(std::move(key));
            return leaf;
        }

        int dir(Node *ptr) {
            return ptr->fa->s[1] == ptr;
        }

        Node* bro(Node *ptr) {
            return ptr->fa->s[dir(ptr)^1];
        }

        void update(Node *x) {
            x->size = x->s[0]->size + x->s[1]->size + 1;
        }

        void rotate(Node *x) {
            if (!x->fa) return;
            Node *y = x->fa, *z = y->fa;
            int t=dir(x);
            if (z) z->link(dir(y), x);
            else root = x;
            y->link(t, x->s[t^1]);
            x->link(t^1, y);

            update(y);
            update(x);
        }

        Node* findNext(Node *x) {
            if (x->s[1]==nil) {
                root->fa = nil;
                for (; x->fa!=nil && dir(x)==1; x=x->fa);
                return x->fa;
            }
            x = x->s[1];
            for (; x->s[0]!=nil; x=x->s[0]);
            return x;
        }

        Node* findPrev(Node *x) {
            if (x->s[0]==nil) {
                root->fa = nil;
                for (; x->fa!=nil && dir(x)==0; x=x->fa);
                return x->fa;
            }
            x = x->s[0];
            for (; x->s[1]!=nil; x=x->s[1]);
            return x;
        }

        std::pair<Node*, int> findEmplacePos(Node *x, const Key &key) {
            if (cmp(key, *x->key)) {
                return x->s[0]==nil ? std::make_pair(x, 0) : findEmplacePos(x->s[0], key);
            } else if (cmp(*x->key, key)) {
                return x->s[1]==nil ? std::make_pair(x, 1) : findEmplacePos(x->s[1], key);
            } else return std::make_pair(x, -1);
        }

        void maintainEmplace(Node *x) {
            for (;;) {
                // Case 2: x->fa is root or black
                if (!x->fa || root == x->fa || x->fa->black) return;
                // Case 3: x->fa->bro is red
                Node *unc = bro(x->fa);
                if (!unc->black) {
                    x->fa->black = unc->black = true;
                    x = x->fa->fa;
                } else {
                    // Case 4: dir(x->fa) != dir(x)
                    if (dir(x->fa) != dir(x)) {
                        Node *tmp = x->fa;
                        rotate(x);
                        x = tmp;
                    }

                    // Case 5: dir(x->fa) == dir(x)
                    rotate(x->fa);
                    x->fa->s[0]->black = x->fa->s[1]->black = false;
                    x->fa->black = true;
                    return;
                }
            }
        }

        void maintainErase(Node *x) {
            for (; x != root;) {
                Node *b = bro(x);
                // Case 1: bro is red
                if (!b->black) {
                    rotate(b);
                    x->fa->black = false;
                    b->black = true;
                    continue;
                }
                // Case 2: bro and its children are both black and the parent is red
                if (b->black && b->s[0]->black && b->s[1]->black) {
                    x->fa->black = true;
                    b->black = false;
                    return;
                }
                // Case 3: bro and its children and the parent are all black
                if (b->black && b->s[0]->black && b->s[1]->black && x->fa->black) {
                    b->black = false;
                    x = x->fa;
                    continue;
                }

                Node *cnep = b->s[dir(x)], *dnep = b->s[dir(x)^1];
                // Case 4: bro is black , close nephew is red, and distant nephew is black
                if (b->black && !cnep->black && dnep->black) {
                    rotate(cnep);
                    cnep->black = true;
                    b->black = false;
                    continue;
                }
                // Case 5: bro is black , close nephew is black, and distant nephew is red
                if (b->black && cnep->black && !dnep->black) {
                    rotate(b);
                    dnep->black = true;
                    std::swap(b->black, x->fa->black);
                    return;
                }
            }
        }

        void updateToRoot(Node *x) {
            for (; x!=root; x = x->fa) update(x);
            update(root);
        }

    public:

        class iterator {
            friend class ESet<Key, Compare>;
        private:
            ESet *from;
            Node *ptr;

            iterator(Node *ptr, ESet *from) : ptr{ptr}, from{from} {}

        public:
            iterator(const iterator &other) : ptr{other.ptr}, from{other.from} {}

            const Key& operator*() const { return *ptr->key; }
            const Key* operator->() const { return ptr->key; }
            
            virtual iterator operator++(int) {
                iterator tmp = *this;
                ptr = from->findNext(ptr);
                return tmp;
            }

            virtual iterator operator--(int) {
                iterator tmp = *this;
                ptr = from->findPrev(ptr);
                return tmp;
            }

            virtual iterator& operator++() {
                ptr = from->findNext(ptr);
                return *this;
            }

            virtual iterator& operator--() {
                ptr = from->findPrev(ptr);
                return *this;
            }

            bool operator==(const iterator &other) const {
                return ptr == other.ptr;
            }

            bool operator!=(const iterator &other) const {
                return ptr!= other.ptr;
            }
        };

        class const_iterator : iterator {
            friend class ESet<Key, Compare>;
        private:
            const_iterator(Node *ptr, ESet from) : iterator(ptr, from) {}

        public:
            const_iterator(const iterator &other) : iterator(other) {}

            virtual const_iterator operator++(int) {
                return const_iterator(iterator::operator++(0));
            }
            virtual const_iterator operator--(int) {
                return const_iterator(iterator::operator--(0));
            }
            virtual const_iterator& operator++() {
                return const_iterator(iterator::operator++());
            }
            virtual const_iterator& operator--() {
                return const_iterator(iterator::operator--());
            }
        };

        ESet() : root{nullptr}, nil{new Node} {
            nil->black = true;
            root = nil;
        }

        ~ESet() {
            clear();
            delete nil;
        }

        ESet(const ESet &other) : root{nullptr}, nil(new Node) {
            root = clone(other.root, other);
        }

        ESet& operator=(const ESet &other) {
            if (&other == this) return *this;
            clear();
            root = clone(other.root, other);
        }

        ESet(ESet &&other) noexcept : root{std::move(other.root)}, nil{std::move(other.nil)} {
           other.root = other.nil = nullptr;
        }
        
        ESet& operator=(ESet &&other) noexcept {
            if (&other == this) return *this;
            clear();
            root = std::move(other.root);
            nil = std::move(other.nil);
            other.root = other.nil = nullptr;
        }
        
        template <class... Args>
        std::pair<iterator, bool> emplace(Args &&... args) {
            Key tar(std::forward<Args>(args)...);
            //Case 1: empty
            if (root==nil) {
                root = newLeaf(std::move(tar));
                return std::make_pair(iterator(root, this), true);
            }

            Node *p;
            int flag;
            std::tie(p, flag) = findEmplacePos(root, tar);
            if (flag<0) return std::make_pair(iterator(p, this), false);
            p->link(flag, newLeaf(std::move(tar)));

            maintainEmplace(p);
            return std::make_pair(iterator(p->s[flag], this), true);
        }

        size_t erase(const Key &key) {
            Node *x = find(key).ptr, *y;
            // Not exist
            if (x == nil) return 0;
            root->black = true;
            // In case that x has two children
            if (x->s[0] != nil && x->s[1] != nil) {
                y = findNext(x);
                std::swap(x->key, y->key);
                x = y;
            }

            // Case A: x has only one child
            // At this time, x->s must be red and x must be black
            if (x->s[0] != nil || x->s[1] != nil) {
                y = x->s[0] != nil ? x->s[0] : x->s[1];
                y->black = true;
                if (x == root) {
                    root = y;
                    y->fa = nil;
                } else {
                    x->fa->link(dir(x), y);
                    updateToRoot(x->fa);
                }
                delete x;
                return 1;
            }

            // Case B: x has no child
            // Case B.0: x is root
            if (x == root) {
                root = nil;
                delete x;
                return 1;
            }
            // Case B.1: x is red
            if (!x->black) {
                x->fa->link(dir(x), nil);
                updateToRoot(x->fa);
                delete x;
                return 1;
            }
            // Case B.2: x is black
            if (x->black) {
                maintainErase(x);
                x->fa->link(dir(x), nil);
                delete x;
                return 1;
            }

            // Error if reaching here
            return -1;
        }

        iterator find(const Key &key) {
            Node *p = root;
            for (; p!=nil; ) {
                if (cmp(key, *p->key)) {
                    p = p->s[0];
                } else if (cmp(*p->key, key)) {
                    p = p->s[1];
                } else return iterator(p, this);
            }
            return end();
        }

        const_iterator find(const Key &key) const {
            Node *p = root;
            for (; p!=nil; ) {
                if (cmp(key, p->key)) {
                    p = p->s[0];
                } else if (cmp(p->key, key)) {
                    p = p->s[1];
                } else return const_iterator(p, this);
            }
            return cend();
        }

        void clear() noexcept {
            recollect(root);
        }

        size_t range(const Key &l, const Key &r) {
            if (cmp(r, l)) return 0;
            Node *pl = lower_bound(l).ptr, *pr = upper_bound(r).ptr;
            if (pr == nil) {
                if (pl == nil) return 0;
                else return size()-pl->size+1;
            }
            return pr->size-pl->size;
        }

        size_t size() const noexcept {
            return root->size;
        }

        iterator lower_bound(const Key &key) {
            Node *p = root;
            for (; p!=nil; ) {
                if (p->s[0]!=nil && !cmp(*p->s[0]->key, key)) {
                    p = p->s[0];
                } else if (!cmp(*p->key, key)) {
                    return iterator(p, this);
                } else p = p->s[1];
            }
            return end();
        }

        iterator upper_bound(const Key &key) {
            Node *p = root;
            for (; p!=nil; ) {
                if (p->s[0]!=nil && cmp(key, *p->s[0]->key)) {
                    p = p->s[0];
                } else if (cmp(key, *p->key)) {
                    return iterator(p, this);
                } else p = p->s[1];
            }
            return end();
        }

        iterator begin() noexcept {
            Node *p = root;
            for (; p!=nil && p->s[0]!=nil; p = p->s[0]);
            return iterator(p, this);
        }

        iterator end() noexcept {
            return iterator(nil, this);
        }

        const_iterator cbegin() const noexcept {
            return const_iterator(begin());
        }

        const_iterator cend() const noexcept {
            return const_iterator(end());
        }
    };
};

#endif // ESET_H