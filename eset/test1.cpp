#include <iostream>
#include "eset.hpp"

// Test for basic insert and enumerate
void test1() {
    sjtu::ESet<int> s;
    for (int i=0; i<10; i++) s.emplace(i);
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// Test for insert and erase
void test2() {
    sjtu::ESet<int> s;
    for (int i=0; i<10; i++) s.emplace(i);
    for (int i=0; i<10; i+=2) s.erase(i);
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// Test for other functions
void test3() {
    sjtu::ESet<int> s;
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

int main() {
    test1();
    test2();
    test3();
    return 0;
}