#ifndef CIRCULAR_BUFFER_TESTS_FIXTURES_H
#define CIRCULAR_BUFFER_TESTS_FIXTURES_H

#include <circular_buffer.h>
#include <gtest/gtest.h>
#include <string>

// ==================== Shared Test Fixtures ====================
// Wynesены сюда, потому что TYPED_TEST(...) в разных .cpp-файлах (разных
// единицах трансляции) должен видеть один и тот же класс фикстуры и его
// регистрацию через TYPED_TEST_SUITE. Классы-шаблоны и typedef'ы можно
// безопасно определять в заголовке, подключаемом в несколько .cpp — это не
// нарушает ODR.

template <typename T>
class CircularBufferIntTest : public testing::Test {};

using CircularBufferIntTypes = testing::Types<
    circular_buffer<int, false>
    #ifdef RUN_EXT_TESTS
    , circular_buffer<int, true>
    #endif
>;
TYPED_TEST_SUITE(CircularBufferIntTest, CircularBufferIntTypes);

template <typename T>
class CircularBufferStringTest : public testing::Test {};

using CircularBufferStringTypes = testing::Types<
    circular_buffer<std::string, false>
    #ifdef RUN_EXT_TESTS
    , circular_buffer<std::string, true>
    #endif
>;
TYPED_TEST_SUITE(CircularBufferStringTest, CircularBufferStringTypes);

#endif // CIRCULAR_BUFFER_TESTS_FIXTURES_H
