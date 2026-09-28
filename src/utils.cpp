#ifndef UTILS
#define UTILS

#include <random>

template <typename T> T random(T min, T max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(gen);
    } else if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    } else {
        static_assert(
            std::is_integral<T>::value || std::is_floating_point<T>::value,
            "randomBetween only supports integral or floating point types");
    }
}

#endif // UTILS