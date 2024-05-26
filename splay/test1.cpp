#include "eset.hpp"
#include <set>
#include <iostream>
#include <random>

// using namespace splay;

// Test for basic insert and enumerate
void test1() {
    std::cout << "test1:" << std::endl;
    ESet<int> s;
    for (int i=0; i<10; i++) {
        s.emplace(i);
#ifdef DEBUG
        s.debug_print();
#endif
    }
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
#ifdef DEBUG
        s.debug_print();
#endif
    }
    std::cout << std::endl;
}

// Test for insert and erase
void test2() {
    std::cout << "test2:" << std::endl;
    ESet<int> s;
    for (int i=0; i<10; i++) s.emplace(i);
    for (int i=0; i<10; i+=2) s.erase(i);
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// Test for other functions
void test3() {
    std::cout << "test3:" << std::endl;
    ESet<int> s, s1, s2;
    for (int i=0; i<10; i++) s.emplace(i);
    for (int i=0; i<10; i+=2) s.erase(i);
    std::cout << s.size() << std::endl;
    auto itl = s.lower_bound(3), itr = s.upper_bound(6);
    std::cout << *itl << " " << *itr << std::endl;
    std::cout << s.range(3, 8) << std::endl;
    s.emplace(4);
    std::cout << *(++itl) << " " << *(--itr) << std::endl;
    std::cout << s.range(3, 8) << std::endl;
    itl = s.begin(), itr = s.end();
    std::cout << ((--itl) == s.begin()) << " " << *itl << std::endl;
    // std::cout << ((++itr) == s.end()) << " " << *itr << std::endl;
    s.erase(3);
    std::cout << ((--itl) == s.begin()) << " " << *itl << std::endl;
    std::cout << *(++itl); 
    std::cout << " " << *(--itr) << std::endl;

    s1 = s;
    s2 = std::move(s);
    for (auto it = s1.begin(); it!= s1.end(); ++it) {
        std::cout << *it << " ";
    }
    for (auto it = s2.begin(); it!= s2.end(); ++it) {
        std::cout << *it << " ";
    }
}

// Randomly insert and erase
void test4() {
    std::cout << "test4:" << std::endl;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 10);
    std::bernoulli_distribution dist2(0.5);
    ESet<int> s, s_;
    for (int i=0; i<100000; i++) {
        if (dist2(rng)) {
            int x = dist(rng);
            std::cerr << "insert " << x << std::endl;
            s.emplace(x);
        } else {
            int x = dist(rng);
            std::cerr << "erase " << x << std::endl;
            s.erase(x);
        }
        s_ = s;
        s = std::move(s_);
        s_ = ESet<int>();
        auto iter = s.begin();
        for (; iter!= s.end(); ++iter) {
            std::cout << *iter << " ";
            if (s.find(*iter) != iter) {
                std::cerr << "error" << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

// Randomly insert and erase with checking lower_bound and upper_bound
void test6() {
    std::cout << "test6:" << std::endl;
    const int M = 20;
    ESet<int> s1;
    std::set<int> s2;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, M);
    std::bernoulli_distribution dist2(0.5);
    for (int i=0; i<100000; i++) {
        int x = dist(rng);
        if (dist2(rng)) {
            s1.emplace(x), s2.emplace(x);
        } else {
            s1.erase(x), s2.erase(x);
        }

        for (int j=0; j<=M; j++) {
            int u, v;
            if (s1.lower_bound(j)== s1.end()) {
                if (s2.lower_bound(j) != s2.end()) {
                    std::cout << "error" << std::endl;
                }
            } else {
                if (*s1.lower_bound(j) != *s2.lower_bound(j)) {
                    std::cout << "error" << std::endl;
                }
            }

            if (s1.upper_bound(j)== s1.end()) {
                if (s2.upper_bound(j) != s2.end()) {
                    std::cout << "error" << std::endl;
                }
            } else {
                if ((u=*s1.upper_bound(j)) != (v=*s2.upper_bound(j))) {
                    std::cout << "error" << std::endl;
                }
            }
        }
    }
}

void test7() {
    std::cout << "test7:" << std::endl;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 10);
    std::bernoulli_distribution dist2(0.5);
    ESet<int> s1;
    std::set<int> s2;

    for (int i=1; i<=100000; i++) {
        int u, v;
        if (u=dist2(rng)) {
            v = dist(rng);
            s1.emplace(v), s2.emplace(v);
        } else {
            v = dist(rng);
            s1.erase(v), s2.erase(v);
        }

        auto x = s1.begin();
        auto y = s2.begin();
        for (; x!=s1.end() && y!=s2.end(); ++x, ++y) {
            if (*x!= *y) {
                std::cout << "error" << std::endl;
            }
        }
        if (x!= s1.end() || y!= s2.end()) {
            std::cout << "error" << std::endl;
        }
    }
}

struct Int {
    int v;
};

struct IntComp {
    bool operator()(const Int& a, const Int& b) const {
        return a.v < b.v;
    }
};


// // Randomly insert and erase
// void test5() {
//     std::mt19937 rng(std::random_device{}());
//     std::uniform_int_distribution<int> dist(0, 20);
//     std::bernoulli_distribution dist2(0.5);
//     ESet<Int, IntComp> s;
//     for (int i=0; i<10000; i++) {
//         if (dist2(rng)) {
//             int x = dist(rng);
//             std::cerr << "insert " << x << std::endl;
//             s.emplace(Int{x});
//         } else {
//             int x = dist(rng);
//             std::cerr << "erase " << x << std::endl;
//             s.erase(Int{x});
//         }
//         auto iter = s.begin();
//         for (; iter!= s.end(); ++iter) {
//             std::cout << iter->v << " ";
//         }
//         std::cout << std::endl;
//     }
// }

int main() {
    test1();
    test2();
    test3();
    test4();
    // test5();
    test6();
    test7();
    return 0;
}