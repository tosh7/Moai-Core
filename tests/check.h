#ifndef CHECK_H
#define CHECK_H

#include <cmath>
#include <iostream>
#include <string>

inline int checks = 0;
inline int failures = 0;

#define CHECK_EQ(label, actual, expected)                                     \
    do {                                                                      \
        ++checks;                                                             \
        auto a = (actual);                                                    \
        auto e = (expected);                                                  \
        if (a != e) {                                                         \
            ++failures;                                                       \
            std::cout << "FAIL: " << label << " - expected " << e             \
                      << ", got " << a << std::endl;                          \
        }                                                                     \
    } while (0)

// Float results drift as a run accumulates, so physics compares within a
// tolerance the caller picks rather than for exact equality.
#define CHECK_NEAR(label, actual, expected, tolerance)                        \
    do {                                                                      \
        ++checks;                                                             \
        double a = (actual);                                                  \
        double e = (expected);                                                \
        if (std::abs(a - e) > (tolerance)) {                                  \
            ++failures;                                                       \
            std::cout << "FAIL: " << label << " - expected " << e             \
                      << " +/- " << (tolerance) << ", got " << a              \
                      << std::endl;                                           \
        }                                                                     \
    } while (0)

#define CHECK_TRUE(label, actual)                                             \
    do {                                                                      \
        ++checks;                                                             \
        if (!(actual)) {                                                      \
            ++failures;                                                       \
            std::cout << "FAIL: " << label << std::endl;                      \
        }                                                                     \
    } while (0)

#endif
