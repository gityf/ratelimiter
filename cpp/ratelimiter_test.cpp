#include <iostream>
#include "ratelimiter.hpp"

int main(int argc, char *argv[]) {
    RateLimiter rateLimiter(100, 1000);
    for(int i = 0; i < 100; i++) {
        if (!rateLimiter.Allow()) {
            std::cout << "Allow false, want true." << std::endl;
        }
    }
    if (rateLimiter.Allow()) {
        std::cout << "Allow true, want false." << std::endl;
    }
    if (!rateLimiter.Allow()) {
        std::cout << "Allow false, want false." << std::endl;
    }
    return 0;
}