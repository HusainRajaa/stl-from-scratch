#pragma once

#include <chrono>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace stl::test {

class TestFailure : public std::exception {
public:
    TestFailure(std::string message, const char* file, int line)
        : message_(std::move(message)), file_(file), line_(line) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

    const char* file() const noexcept { return file_; }
    int line() const noexcept { return line_; }

private:
    std::string message_;
    const char* file_;
    int line_;
};

struct TestCase {
    std::string name;
    std::string suite;
    std::function<void()> func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void register_test(std::string suite, std::string name, std::function<void()> func) {
        tests_.push_back({std::move(name), std::move(suite), std::move(func)});
    }

    int run_all() {
        std::cout << "\n========================================\n";
        std::cout << "          STL TEST SUITE RUNNER         \n";
        std::cout << "========================================\n\n";

        std::size_t passed = 0;
        std::size_t failed = 0;
        auto total_start = std::chrono::high_resolution_clock::now();

        for (const auto& test : tests_) {
            std::cout << "[ RUN      ] " << test.suite << "." << test.name << "\n";
            auto start = std::chrono::high_resolution_clock::now();
            bool test_passed = false;
            try {
                test.func();
                test_passed = true;
            } catch (const TestFailure& tf) {
                std::cout << "  \033[31m[FAILED]\033[0m " << tf.file() << ":" << tf.line() << "\n"
                          << "           " << tf.what() << "\n";
            } catch (const std::exception& ex) {
                std::cout << "  \033[31m[FAILED]\033[0m Unexpected exception: " << ex.what() << "\n";
            } catch (...) {
                std::cout << "  \033[31m[FAILED]\033[0m Unknown exception thrown.\n";
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            if (test_passed) {
                ++passed;
                std::cout << "  \033[32m[       OK ]\033[0m " << test.suite << "." << test.name
                          << " (" << duration_us / 1000.0 << " ms)\n";
            } else {
                ++failed;
                std::cout << "  \033[31m[  FAILED  ]\033[0m " << test.suite << "." << test.name
                          << " (" << duration_us / 1000.0 << " ms)\n";
            }
        }

        auto total_end = std::chrono::high_resolution_clock::now();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();

        std::cout << "\n========================================\n";
        std::cout << "Summary: " << passed << " passed, " << failed << " failed (Total " << tests_.size() << ")\n";
        std::cout << "Total execution time: " << total_ms << " ms\n";
        std::cout << "========================================\n\n";

        return failed == 0 ? 0 : 1;
    }

private:
    std::vector<TestCase> tests_;
};

struct TestRegistrar {
    TestRegistrar(std::string suite, std::string name, std::function<void()> func) {
        TestRegistry::instance().register_test(std::move(suite), std::move(name), std::move(func));
    }
};

template <typename T>
std::string to_string_repr(const T& val) {
    if constexpr (std::is_convertible_v<T, std::string_view>) {
        return std::string("\"") + std::string(val) + "\"";
    } else {
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }
}

template <typename T1, typename T2>
bool are_equal(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_equal(a, b);
    } else {
        return a == b;
    }
}

template <typename T1, typename T2>
bool are_not_equal(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_not_equal(a, b);
    } else {
        return a != b;
    }
}

template <typename T1, typename T2>
bool is_less(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_less(a, b);
    } else {
        return a < b;
    }
}

template <typename T1, typename T2>
bool is_less_equal(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_less_equal(a, b);
    } else {
        return a <= b;
    }
}

template <typename T1, typename T2>
bool is_greater(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_greater(a, b);
    } else {
        return a > b;
    }
}

template <typename T1, typename T2>
bool is_greater_equal(const T1& a, const T2& b) {
    if constexpr (std::is_integral_v<T1> && std::is_integral_v<T2>) {
        return std::cmp_greater_equal(a, b);
    } else {
        return a >= b;
    }
}

} // namespace stl::test

#define TEST_CASE(suite_name, test_name) \
    static void suite_name##_##test_name(); \
    static ::stl::test::TestRegistrar suite_name##_##test_name##_registrar( \
        #suite_name, #test_name, &suite_name##_##test_name); \
    static void suite_name##_##test_name()

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed: ") + #condition, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed (expected false): ") + #condition, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_EQ(expected, actual) \
    do { \
        const auto& exp_val = (expected); \
        const auto& act_val = (actual); \
        if (!::stl::test::are_equal(exp_val, act_val)) { \
            throw ::stl::test::TestFailure( \
                std::string("Expected: ") + ::stl::test::to_string_repr(exp_val) + \
                ", Actual: " + ::stl::test::to_string_repr(act_val) + \
                " (" #expected " == " #actual ")", __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_NE(val1, val2) \
    do { \
        const auto& v1 = (val1); \
        const auto& v2 = (val2); \
        if (!::stl::test::are_not_equal(v1, v2)) { \
            throw ::stl::test::TestFailure( \
                std::string("Expected not equal: ") + #val1 " == " #val2, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_LT(val1, val2) \
    do { \
        if (!::stl::test::is_less((val1), (val2))) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed: ") + #val1 " < " #val2, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_LE(val1, val2) \
    do { \
        if (!::stl::test::is_less_equal((val1), (val2))) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed: ") + #val1 " <= " #val2, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_GT(val1, val2) \
    do { \
        if (!::stl::test::is_greater((val1), (val2))) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed: ") + #val1 " > " #val2, __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_GE(val1, val2) \
    do { \
        if (!::stl::test::is_greater_equal((val1), (val2))) { \
            throw ::stl::test::TestFailure( \
                std::string("Assertion failed: ") + #val1 " >= " #val2, __FILE__, __LINE__); \
        } \
    } while (false)
#define ASSERT_THROWS(expression, exception_type) \
    do { \
        bool caught = false; \
        try { \
            (void)(expression); \
        } catch (const exception_type&) { \
            caught = true; \
        } catch (...) { \
            throw ::stl::test::TestFailure( \
                std::string("Expression " #expression " threw different exception type than " #exception_type), \
                __FILE__, __LINE__); \
        } \
        if (!caught) { \
            throw ::stl::test::TestFailure( \
                std::string("Expected " #expression " to throw " #exception_type ", but no exception thrown"), \
                __FILE__, __LINE__); \
        } \
    } while (false)

#define ASSERT_NO_THROW(expression) \
    do { \
        try { \
            (void)(expression); \
        } catch (const std::exception& e) { \
            throw ::stl::test::TestFailure( \
                std::string("Expected no exception from " #expression ", but caught: ") + e.what(), \
                __FILE__, __LINE__); \
        } catch (...) { \
            throw ::stl::test::TestFailure( \
                std::string("Expected no exception from " #expression ", but caught unknown exception"), \
                __FILE__, __LINE__); \
        } \
    } while (false)

#define RUN_ALL_TESTS() ::stl::test::TestRegistry::instance().run_all()
