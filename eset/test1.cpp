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

int main() {
    test1();
    test2();
    return 0;
}