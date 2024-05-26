#include "eset.hpp"
#include <iostream>
#include <ctime>

const unsigned int M = 100000;

unsigned int myrand() {
    static unsigned int n = 1;
    n = (n << 13) ^ n;
    n = (n >> 17) ^ n;
    n = (n << 5) ^ n;
    return n%M;
}

void test1() {
    std::cout << "test1" << std::endl;
    ESet<unsigned int> s, s_;
    time_t start = clock();
    for (int i=1; i<=2*M; i++) {
        s.emplace(myrand());
    }
    time_t end = clock();
    std::cout << "Emplace: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << std::endl;

    start = clock();
    for (int i=1; i<=2*M; i++) {
        s.erase(myrand());
    }
    end = clock();
    std::cout << "Erase: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << std::endl;

    for (int i=1; i<=2*M; i++) {
        s.emplace(myrand());
    }

    std::cout << "Size: " << s.size() << std::endl;
    start = clock();
    for (int i=1; i<=100; i++) {
        s_ = s;
    }
    end = clock();
    std::cout << "Copy: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << std::endl;

    start = clock();
    unsigned int cnt = 0;
    for (int i=1; i<=2*M; i++) {
        cnt += (s.find(myrand()) != s.end());
    }
    end = clock();
    std::cout << "Count: " << cnt << std::endl;
    std::cout << "Find: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << std::endl;

    start = clock();
    unsigned int y = 0;
    for (auto it = s.begin(); it!= s.end(); ++it) {
        y += *it;
    }
    end = clock();
    std::cout << "Sum: " << y << std::endl;
    std::cout << "Enumeration: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << std::endl;
}

void test2() {
}

int main() {
    test1();
    return 0;
}