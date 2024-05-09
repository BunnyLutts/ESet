#include "eset.hpp"
#include <iostream>
#include <random>

// Test for basic insert and enumerate
void test1() {
    ESet<int> s;
    for (int i=0; i<10; i++) s.emplace(i);
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// Test for insert and erase
void test2() {
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
    ESet<int> s;
    for (int i=0; i<10; i++) s.emplace(i);
    for (int i=0; i<10; i+=2) s.erase(i);
    std::cout << s.size() << std::endl;
    auto itl = s.lower_bound(3), itr = s.upper_bound(6);
    std::cout << *itl << " " << *itr << std::endl;
    std::cout << s.range(3, 8) << std::endl;
    s.emplace(4);
    std::cout << *(++itl) << " " << *(--itr) << std::endl;
    std::cout << s.range(3, 8) << std::endl;
}

// Randomly insert and erase
void test4() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 20);
    std::bernoulli_distribution dist2(0.5);
    ESet<int> s;
    for (int i=0; i<10000; i++) {
        if (dist2(rng)) {
            int x = dist(rng);
            std::cerr << "insert " << x << std::endl;
            s.emplace(x);
        } else {
            int x = dist(rng);
            std::cerr << "erase " << x << std::endl;
            s.erase(x);
        }
    }
}

int main() {
    test1();
    test2();
    test3();
    test4();
    return 0;
}