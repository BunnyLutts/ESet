#ifndef ESET_H

#define ESET_H

// #include <functional>
// #include <exception>
#include <stdexcept>
#ifdef DEBUG
#include <iostream>
#endif

template <class Key, class Compare = std::less<Key> >
class ESet {
private:
    struct Node {
        Node *s[2], *fa;
        Key *key;
        size_t size;
        bool black;

        //Initially black
        Node() : s{nullptr, nullptr}, fa{nullptr}, key{nullptr}, size{0}, black{true} {}

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

    Node* clone(const Node *src, const ESet &src_set) {
        if (src == src_set.nil) return nil;
        Node *dest = new Node;
        dest->black = src->black;
        dest->size = src->size;
        dest->key = new Key(*src->key);
        dest->link(0, clone(src->s[0], src_set));
        dest->link(1, clone(src->s[1], src_set));
        return dest;
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

    int dir(const Node *ptr) const {
        return ptr->fa->s[1] == ptr;
    }

    Node* bro(const Node *ptr) const {
        return ptr->fa->s[dir(ptr)^1];
    }

    void update(Node *x) {
        x->size = x->s[0]->size + x->s[1]->size + 1;
    }

    void rotate(Node *x) {
        if (x==root) return;
        Node *y = x->fa, *z = y->fa;
        int t=dir(x);
        if (y!=root) z->link(dir(y), x);
        else root = x;
        y->link(t, x->s[t^1]);
        x->link(t^1, y);

        update(y);
        update(x);

        #ifdef DEBUG
        // std::cerr << "Rotate " << x->key << " " << y->key << std::endl;
        // debug_print(root, 1);
        #endif
    }

    Node* findNext(Node *x) {
        if (x==nil) return x;
        if (x->s[1]==nil) {
            root->fa = nil;
            for (; x->fa!=nil && dir(x)==1; x=x->fa);
            return x->fa;
        }
        x = x->s[1];
        for (; x->s[0]!=nil; x=x->s[0]);
        return x;
    }

    const Node* findNext(const Node *x) const {
        if (x==nil) return x;
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
        if (x==nil) return x;
        if (x->s[0]==nil) {
            Node *y = x;
            root->fa = nil;
            for (; y->fa!=nil && dir(y)==0; y=y->fa);
            return y->fa==nil ? x : y->fa;
        }
        x = x->s[0];
        for (; x->s[1]!=nil; x=x->s[1]);
        return x;
    }

    const Node* findPrev(const Node *x) const {
        if (x==nil) return x;
        if (x->s[0]==nil) {
            const Node *y = x;
            root->fa = nil;
            for (; y->fa!=nil && dir(y)==0; y=y->fa);
            return y->fa==nil ? x : y->fa;
        }
        x = x->s[0];
        for (; x->s[1]!=nil; x=x->s[1]);
        return x;
    }
    std::pair<Node*, int> findEmplacePos(Node *x, const Key &key) const {
        if (cmp(key, *x->key)) {
            return x->s[0]==nil ? std::make_pair(x, 0) : findEmplacePos(x->s[0], key);
        } else if (cmp(*x->key, key)) {
            return x->s[1]==nil ? std::make_pair(x, 1) : findEmplacePos(x->s[1], key);
        } else return std::make_pair(x, -1);
    }

    const Node* findLast() const {
        Node *x = root;
        for (; x!=nil && x->s[1]!=nil; x=x->s[1]);
        return x;
    }

    void maintainEmplace(Node *x) {
        for (;;) {
            // Case 2: x->fa is root or black
            if (x==root || root == x->fa || x->fa->black) return;
            // Case 3: x->fa->bro is red
            Node *unc = bro(x->fa);
            if (!unc->black) {
                x->fa->black = unc->black = true;
                x = x->fa->fa;
                x->black = false;
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
            if (b->black && b->s[0]->black && b->s[1]->black && !x->fa->black) {
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
            // Case 5: bro is black, and distant nephew is red
            if (b->black && !dnep->black) {
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

    #ifdef DEBUG
    void debug_print(Node *ptr, int x) const {
        if (ptr==nil) return;
        std::cerr << x << ": " << (ptr->black? "black" : "red") << ", size: " << ptr->size << ", key: " << *ptr->key << "\n";
        debug_print(ptr->s[0], x*2);
        debug_print(ptr->s[1], x*2+1);
    }
    #endif

    Node* nfind(const Key &key) {
        Node *p = root;
        for (; p!=nil; ) {
            if (cmp(key, *p->key)) {
                p = p->s[0];
            } else if (cmp(*p->key, key)) {
                p = p->s[1];
            } else return p;
        }
        return nil;
    }

    const Node* nfind(const Key &key) const {
        Node *p = root;
        for (; p!=nil; ) {
            if (cmp(key, *p->key)) {
                p = p->s[0];
            } else if (cmp(*p->key, key)) {
                p = p->s[1];
            } else return p;
        }
        return nil;
    }

public:

    class iterator {
        friend class ESet<Key, Compare>;
    private:
        const ESet *from;
        const Key *key;

        iterator(const Node *ptr, const ESet *from) : key{ptr->key}, from{from} {}
        iterator(const Key *key, const ESet *from) : key{key}, from{from} {}

        const Node* find() {
            return key ? from->nfind(*key) : from->nil;
        }

    public:
        iterator() : key{nullptr}, from{nullptr} {}

        const Key& operator*() const { 
            if (!key) throw std::out_of_range("Out of range");
            return *key; 
        }

        const Key* operator->() const { 
            if (!key) throw std::out_of_range("Out of range");
            return key; 
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            const Node *ptr = find();
            key = from->findNext(ptr)->key;
            return tmp;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            const Node *ptr = find();
            key = ptr == from->nil ? from->findLast()->key : from->findPrev(ptr)->key;
            return tmp;
        }

        iterator& operator++() {
            const Node *ptr = find();
            key = from->findNext(ptr)->key;
            return *this;
        }

        iterator& operator--() {
            const Node *ptr = find();
            key = ptr == from->nil ? from->findLast()->key : from->findPrev(ptr)->key;
            return *this;
        }

        bool operator==(const iterator &other) const {
            return from == other.from && key == other.key;
        }

        bool operator!=(const iterator &other) const {
            return from != other.from || key != other.key;
        }
    };

    // class const_iterator : iterator {
    //     friend class ESet<Key, Compare>;
    // private:
    //     const_iterator(Node *ptr, ESet from) : iterator(ptr, from) {}

    // public:
    //     const_iterator(const iterator &other) : iterator(other) {}

    //     virtual const_iterator operator++(int) {
    //         return const_iterator(iterator::operator++(0));
    //     }
    //     virtual const_iterator operator--(int) {
    //         return const_iterator(iterator::operator--(0));
    //     }
    //     virtual const_iterator& operator++() {
    //         return const_iterator(iterator::operator++());
    //     }
    //     virtual const_iterator& operator--() {
    //         return const_iterator(iterator::operator--());
    //     }
    // };

    ESet() : root{nullptr}, nil{new Node} {
        nil->black = true;
        root = nil;
    }

    ~ESet() {
        clear();
        if (nil != nullptr) delete nil;
    }

    ESet(const ESet &other) : root{nullptr}, nil(new Node) {
        root = clone(other.root, other);
    }

    ESet& operator=(const ESet &other) {
        if (&other == this) return *this;
        clear();
        root = clone(other.root, other);
        return *this;
    }

    ESet(ESet &&other) noexcept : root{std::move(other.root)}, nil{std::move(other.nil)} {
        other.root = other.nil = new Node;
    }
    
    ESet& operator=(ESet &&other) noexcept {
        if (&other == this) return *this;
        clear();
        if (nil) delete nil;
        root = std::move(other.root);
        nil = std::move(other.nil);
        other.root = other.nil = nullptr;
        return *this;
    }
    
    template <class... Args>
    std::pair<iterator, bool> emplace(Args &&... args) {
        Key tar(std::forward<Args>(args)...);
        //Case 1: empty
        if (root==nil) {
            root = newLeaf(std::move(tar));
            return std::make_pair(iterator(root, this), true);
        }

        Node *p, *np;
        int flag;
        auto temp = findEmplacePos(root, tar);
        p = temp.first;
        flag = temp.second;
        if (flag<0) return std::make_pair(iterator(p, this), false);
        p->link(flag, np = newLeaf(std::move(tar)));

        updateToRoot(np);
        maintainEmplace(np);

        #ifdef DEBUG
        std::cerr << "After emplace: \n";
        debug_print(root, 1);
        #endif

        return std::make_pair(iterator(np, this), true);
    }

    size_t erase(const Key &key) {
        Node *x = nfind(key), *y;
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
            #ifdef DEBUG
            std::cerr << "After erase: \n";
            debug_print(root, 1);
            #endif
            return 1;
        }

        // Case B: x has no child
        // Case B.0: x is root
        if (x == root) {
            root = nil;
            delete x;
            #ifdef DEBUG
            std::cerr << "After erase: \n";
            debug_print(root, 1);
            #endif
            return 1;
        }
        // Case B.1: x is red
        if (!x->black) {
            x->fa->link(dir(x), nil);
            updateToRoot(x->fa);
            delete x;
            #ifdef DEBUG
            std::cerr << "After erase: \n";
            debug_print(root, 1);
            #endif
            return 1;
        }
        // Case B.2: x is black
        if (x->black) {
            maintainErase(x);
            x->fa->link(dir(x), nil);
            updateToRoot(x->fa);
            delete x;
            #ifdef DEBUG
            std::cerr << "After erase: \n";
            debug_print(root, 1);
            #endif
            return 1;
        }

        // Error if reaching here
        return -1;
    }

    iterator find(const Key &key) const {
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

    void clear() noexcept {
        if (root!=nullptr && root!=nil) recollect(root);
    }

    size_t range(const Key &l, const Key &r) const {
        if (cmp(r, l)) return 0;
        size_t sizel = 0, sizer = 0;
        Node *p;
        for (p = root; p!=nil; ) {
            if (cmp(*p->key, l)) {
                sizel += p->s[0]->size+1;
                p = p->s[1];
            } else p = p->s[0];
        }
        for (p = root; p!=nil; ) {
            if (!cmp(r, *p->key)) {
                sizer += p->s[0]->size+1;
                p = p->s[1];
            } else p = p->s[0];
        }
        return sizer - sizel;
    }

    size_t size() const noexcept {
        return root->size;
    }

    iterator lower_bound(const Key &key) const {
        Node *p = root, *ret = nil;
        for (; p!=nil; ) {
            if (p->s[0]!=nil && !cmp(*p->s[0]->key, key)) {
                p = p->s[0];
            } else if (!cmp(*p->key, key)) {
                ret = p;
                p = p->s[0];
            } else p = p->s[1];
        }
        return iterator(ret, this);
    }

    iterator upper_bound(const Key &key) const {
        Node *p = root, *ret=nil;
        for (; p!=nil; ) {
            if (p->s[0]!=nil && cmp(key, *p->s[0]->key)) {
                p = p->s[0];
            } else if (cmp(key, *p->key)) {
                ret = p;
                p = p->s[0];
            } else p = p->s[1];
        }
        return iterator(ret, this);
    }

    iterator begin() const noexcept {
        Node *p = root;
        for (; p!=nil && p->s[0]!=nil; p = p->s[0]);
        return iterator(p, this);
    }

    iterator end() const noexcept {
        return iterator(nil, this);
    }

    // const_iterator cbegin() const noexcept {
    //     return const_iterator(begin());
    // }

    // const_iterator cend() const noexcept {
    //     return const_iterator(end());
    // }
};

#endif // ESET_H