#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>
#include <list>
#include <numeric>
#include <algorithm>

#include "fixtures.h"

// Helper to force lvalue
template<typename T>
const T& as_lvalue(const T& val) { return val; }

// Helper: создать буфер с достаточной capacity для non-extendable
template<typename CB>
CB make_buffer_with_capacity(typename CB::size_type cap) {
    return CB(cap);
}

// ==================== Original Tests ====================

TYPED_TEST(CircularBufferIntTest, alternatingPush) {
    TypeParam cb(6);
    for (int i = 0; i < 6; ++i) {
        if (i % 2 == 0) {
            cb.push_back(as_lvalue(i));
        } else {
            cb.push_front(as_lvalue(i));
        }
    }
    ASSERT_THAT(cb, testing::ElementsAre(5, 3, 1, 0, 2, 4));
}

TYPED_TEST(CircularBufferStringTest, pushingComplicatedObjects) {
    TypeParam cb(3);
    std::string s1 = "aaa", s2 = "bbb", s3 = "ccc";
    cb.push_back(s1);
    cb.push_back(s2);
    cb.push_back(s3);
    ASSERT_THAT(cb, testing::ElementsAre("aaa", "bbb", "ccc"));
}

TYPED_TEST(CircularBufferIntTest, simplePopTest) {
    TypeParam cb(5);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_front(v4);
    cb.push_front(v5);
    cb.pop_back();
    cb.pop_front();
    ASSERT_THAT(cb, testing::ElementsAre(4, 1, 2));
}

TYPED_TEST(CircularBufferIntTest, popFromEmpty) {
    TypeParam cb(1);
    int v = 1;
    cb.push_back(v);
    cb.pop_back();

    EXPECT_THROW(cb.pop_back(), std::logic_error);
    EXPECT_THROW(cb.pop_front(), std::logic_error);
}

TYPED_TEST(CircularBufferIntTest, eraseOneElement) {
    TypeParam cb = {1, 2, 3, 4, 5};
    cb.erase(cb.cbegin() + 2);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, eraseSequence) {
    TypeParam cb(5);
    for (int i = 0; i < 5; ++i) {
        if (i % 2 == 0) {
            cb.push_back(as_lvalue(i));
        } else {
            cb.push_front(as_lvalue(i));
        }
    }
    cb.erase(cb.cbegin() + 1, cb.cend() - 1);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4));
}

TYPED_TEST(CircularBufferIntTest, nValues) {
    TypeParam cb(5);
    cb.assign(static_cast<size_t>(5), 10);
    ASSERT_THAT(cb, testing::ElementsAre(10, 10, 10, 10, 10));
}

TYPED_TEST(CircularBufferIntTest, assignIterator) {
    TypeParam cb(5);
    std::vector<int> v = {1, 2, 3, 4, 5};
    cb.assign(v.begin() + 1, v.end() - 1);
    ASSERT_THAT(cb, testing::ElementsAre(2, 3, 4));
}

TYPED_TEST(CircularBufferIntTest, initializerList) {
    TypeParam cb = {5, 4, 3, 2, 1};
    cb.assign({1, 2, 3, 4, 5});
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, simpleTest) {
    TypeParam cb(5);
    for (int i = 0; i < 5; ++i) {
        if (i % 2 == 0) {
            cb.push_back(as_lvalue(i));
        } else {
            cb.push_front(as_lvalue(i));
        }
    }
    ASSERT_EQ(cb.front(), 3);
}

// ==================== Construction Tests ====================

TYPED_TEST(CircularBufferIntTest, defaultConstructor) {
    TypeParam cb;
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.size(), 0u);
}

TYPED_TEST(CircularBufferIntTest, capacityConstructor) {
    TypeParam cb(10);
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.capacity(), 10u);
}

TYPED_TEST(CircularBufferIntTest, zeroCapacityConstructor) {
    TypeParam cb(0);
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.capacity(), 0u);
}

TYPED_TEST(CircularBufferIntTest, fillConstructor) {
    TypeParam cb(5, 42);
    EXPECT_EQ(cb.size(), 5u);
    ASSERT_THAT(cb, testing::ElementsAre(42, 42, 42, 42, 42));
}

TYPED_TEST(CircularBufferIntTest, initializerListConstructor) {
    TypeParam cb{1, 2, 3, 4, 5};
    EXPECT_EQ(cb.size(), 5u);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, iteratorRangeConstructor) {
    std::vector<int> v{10, 20, 30, 40};
    // Для non-extendable нужно сначала создать с capacity
    TypeParam cb(v.begin(), v.end());
    EXPECT_EQ(cb.size(), 4u);
    ASSERT_THAT(cb, testing::ElementsAre(10, 20, 30, 40));
}

TYPED_TEST(CircularBufferStringTest, iteratorRangeFromList) {
    std::list<std::string> lst{"hello", "world", "foo"};
    TypeParam cb(lst.begin(), lst.end());
    EXPECT_EQ(cb.size(), 3u);
    ASSERT_THAT(cb, testing::ElementsAre("hello", "world", "foo"));
}

TYPED_TEST(CircularBufferIntTest, copyConstructor) {
    TypeParam original{1, 2, 3};
    TypeParam copy(original);
    EXPECT_EQ(copy, original);
    // Для non-extendable буфера нужна capacity для добавления
    if (copy.capacity() > copy.size()) {
        int v = 4;
        copy.push_back(v);
        EXPECT_NE(copy.size(), original.size());
    }
}

TYPED_TEST(CircularBufferIntTest, copyConstructorEmpty) {
    TypeParam original;
    TypeParam copy(original);
    EXPECT_TRUE(copy.empty());
}

// ==================== Assignment Tests ====================

TYPED_TEST(CircularBufferIntTest, copyAssignment) {
    TypeParam a{1, 2, 3};
    TypeParam b{10, 20};
    b = a;
    EXPECT_EQ(a, b);
}

TYPED_TEST(CircularBufferIntTest, copyAssignmentSelf) {
    TypeParam a{1, 2, 3};
    a = a;
    EXPECT_EQ(a.size(), 3u);
    ASSERT_THAT(a, testing::ElementsAre(1, 2, 3));
}

TYPED_TEST(CircularBufferIntTest, initializerListAssignment) {
    TypeParam cb(10);
    cb = {5, 6, 7, 8};
    ASSERT_THAT(cb, testing::ElementsAre(5, 6, 7, 8));
}

// ==================== Assign Method Tests ====================

TYPED_TEST(CircularBufferIntTest, assignCountValue) {
    TypeParam cb{1, 2, 3};
    cb.assign(static_cast<size_t>(5), 99);
    ASSERT_THAT(cb, testing::ElementsAre(99, 99, 99, 99, 99));
}

TYPED_TEST(CircularBufferIntTest, assignIterRange) {
    std::vector<int> v{10, 20, 30};
    TypeParam cb(10); // Достаточная capacity
    cb.assign(v.begin(), v.end());
    ASSERT_THAT(cb, testing::ElementsAre(10, 20, 30));
}

TYPED_TEST(CircularBufferIntTest, assignInitList) {
    TypeParam cb(10);
    cb.assign({100, 200, 300});
    ASSERT_THAT(cb, testing::ElementsAre(100, 200, 300));
}

TYPED_TEST(CircularBufferIntTest, assignZeroCount) {
    TypeParam cb{1, 2, 3};
    cb.assign(static_cast<size_t>(0), 42);
    EXPECT_TRUE(cb.empty());
}

// ==================== Element Access Tests ====================

TYPED_TEST(CircularBufferIntTest, atAccess) {
    TypeParam cb{10, 20, 30};
    EXPECT_EQ(cb.at(0), 10);
    EXPECT_EQ(cb.at(1), 20);
    EXPECT_EQ(cb.at(2), 30);
}

TYPED_TEST(CircularBufferIntTest, atOutOfRange) {
    TypeParam cb{10, 20};
    EXPECT_THROW(cb.at(2), std::out_of_range);
    EXPECT_THROW(cb.at(100), std::out_of_range);
}

TYPED_TEST(CircularBufferIntTest, atConstOutOfRange) {
    const TypeParam cb{10, 20, 30};
    EXPECT_EQ(cb.at(0), 10);
    EXPECT_THROW(cb.at(3), std::out_of_range);
}

TYPED_TEST(CircularBufferIntTest, subscriptModify) {
    TypeParam cb{5, 10, 15};
    cb[1] = 42;
    EXPECT_EQ(cb[1], 42);
}

TYPED_TEST(CircularBufferIntTest, frontBack) {
    TypeParam cb{1, 2, 3, 4, 5};
    EXPECT_EQ(cb.front(), 1);
    EXPECT_EQ(cb.back(), 5);
}

TYPED_TEST(CircularBufferIntTest, frontBackConst) {
    const TypeParam cb{10, 20};
    EXPECT_EQ(cb.front(), 10);
    EXPECT_EQ(cb.back(), 20);
}

TYPED_TEST(CircularBufferIntTest, frontBackSingleElement) {
    TypeParam cb{42};
    EXPECT_EQ(cb.front(), 42);
    EXPECT_EQ(cb.back(), 42);
}

// ==================== Capacity Tests ====================

TYPED_TEST(CircularBufferIntTest, emptyCheck) {
    TypeParam cb(10); // Нужна capacity для push
    EXPECT_TRUE(cb.empty());
    int v = 1;
    cb.push_back(v);
    EXPECT_FALSE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, sizeTracking) {
    TypeParam cb(10); // Нужна capacity для push
    EXPECT_EQ(cb.size(), 0u);
    int v1 = 1, v2 = 2;
    cb.push_back(v1);
    EXPECT_EQ(cb.size(), 1u);
    cb.push_back(v2);
    EXPECT_EQ(cb.size(), 2u);
    cb.pop_back();
    EXPECT_EQ(cb.size(), 1u);
}

TYPED_TEST(CircularBufferIntTest, maxSizePositive) {
    TypeParam cb;
    EXPECT_GT(cb.max_size(), 0u);
}

TYPED_TEST(CircularBufferIntTest, reserveGrows) {
    TypeParam cb;
    cb.reserve(100);
    EXPECT_GE(cb.capacity(), 100u);
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, reserveSmallerNoOp) {
    TypeParam cb(50);
    cb.reserve(10);
    EXPECT_EQ(cb.capacity(), 50u);
}

TYPED_TEST(CircularBufferIntTest, reservePreservesData) {
    TypeParam cb{1, 2, 3};
    cb.reserve(100);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}


// ==================== Resize Tests ====================

TYPED_TEST(CircularBufferIntTest, resizeGrowWithValue) {
    TypeParam cb{1, 2};
    cb.resize(5, 99);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 99, 99, 99));
}

TYPED_TEST(CircularBufferIntTest, resizeShrink) {
    TypeParam cb{1, 2, 3, 4, 5};
    cb.resize(2);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2));
}

TYPED_TEST(CircularBufferIntTest, resizeSameSize) {
    TypeParam cb{1, 2, 3};
    cb.resize(3);
    EXPECT_EQ(cb.size(), 3u);
}

TYPED_TEST(CircularBufferIntTest, resizeToZero) {
    TypeParam cb{1, 2, 3};
    cb.resize(0);
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, resizeDefaultValue) {
    TypeParam cb(10); // capacity для resize
    cb.resize(3);
    ASSERT_THAT(cb, testing::ElementsAre(0, 0, 0));
}

// ==================== Push/Pop Tests ====================

TYPED_TEST(CircularBufferIntTest, pushBackPopBackSequence) {
    TypeParam cb(10); // Нужна capacity
    int v1 = 10, v2 = 20, v3 = 30;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    EXPECT_EQ(cb.back(), 30);
    cb.pop_back();
    EXPECT_EQ(cb.back(), 20);
    cb.pop_back();
    EXPECT_EQ(cb.back(), 10);
    cb.pop_back();
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, pushFrontPopFrontSequence) {
    TypeParam cb(10); // Нужна capacity
    int v1 = 10, v2 = 20, v3 = 30;
    cb.push_front(v1);
    cb.push_front(v2);
    cb.push_front(v3);
    EXPECT_EQ(cb.front(), 30);
    cb.pop_front();
    EXPECT_EQ(cb.front(), 20);
    cb.pop_front();
    EXPECT_EQ(cb.front(), 10);
}

TYPED_TEST(CircularBufferIntTest, popBackEmptyThrows) {
    TypeParam cb;
    EXPECT_THROW(cb.pop_back(), std::logic_error);
}

TYPED_TEST(CircularBufferIntTest, popFrontEmptyThrows) {
    TypeParam cb;
    EXPECT_THROW(cb.pop_front(), std::logic_error);
}

TYPED_TEST(CircularBufferIntTest, mixedPushFrontBack) {
    TypeParam cb(10); // Нужна capacity
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    cb.push_back(v3);
    cb.push_front(v2);
    cb.push_back(v4);
    cb.push_front(v1);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TYPED_TEST(CircularBufferStringTest, emplaceBackConstruct) {
    TypeParam cb(10); // Нужна capacity
    auto& ref = cb.emplace_back("hello");
    EXPECT_EQ(ref, "hello");
    cb.emplace_back(5, 'x');
    ASSERT_THAT(cb, testing::ElementsAre("hello", "xxxxx"));
}

TYPED_TEST(CircularBufferStringTest, emplaceFrontConstruct) {
    TypeParam cb(10); // Нужна capacity
    auto& ref = cb.emplace_front("world");
    EXPECT_EQ(ref, "world");
    cb.emplace_front(3, 'a');
    ASSERT_THAT(cb, testing::ElementsAre("aaa", "world"));
}

// ==================== Insert Tests ====================

TYPED_TEST(CircularBufferIntTest, insertSingleBegin) {
    TypeParam cb{2, 3, 4};
    int v = 1;
    auto it = cb.insert(cb.cbegin(), v);
    EXPECT_EQ(*it, 1);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TYPED_TEST(CircularBufferIntTest, insertSingleEnd) {
    TypeParam cb{1, 2, 3};
    int v = 4;
    auto it = cb.insert(cb.cend(), v);
    EXPECT_EQ(*it, 4);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TYPED_TEST(CircularBufferIntTest, insertSingleMiddle) {
    TypeParam cb{1, 3, 4};
    int v = 2;
    auto it = cb.insert(cb.cbegin() + 1, v);
    EXPECT_EQ(*it, 2);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TYPED_TEST(CircularBufferIntTest, insertMultipleCount) {
    TypeParam cb{1, 5};
    cb.insert(cb.cbegin() + 1, static_cast<size_t>(3), 99);
    ASSERT_THAT(cb, testing::ElementsAre(1, 99, 99, 99, 5));
}

TYPED_TEST(CircularBufferIntTest, insertZeroCount) {
    TypeParam cb{1, 2, 3};
    auto it = cb.insert(cb.cbegin() + 1, static_cast<size_t>(0), 42);
    EXPECT_EQ(cb.size(), 3u);
    EXPECT_EQ(*it, 2);
}

TYPED_TEST(CircularBufferIntTest, insertIterRange) {
    TypeParam cb{1, 5};
    std::vector<int> v{2, 3, 4};
    cb.insert(cb.cbegin() + 1, v.begin(), v.end());
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, insertInitList) {
    TypeParam cb{1, 5};
    cb.insert(cb.cbegin() + 1, {2, 3, 4});
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, insertIntoEmpty) {
    TypeParam cb(10); // Нужна capacity для non-extendable
    int v = 42;
    cb.insert(cb.cbegin(), v);
    EXPECT_EQ(cb.size(), 1u);
    EXPECT_EQ(cb[0], 42);
}

TYPED_TEST(CircularBufferIntTest, insertTriggeringExpansion) {
    TypeParam cb(10); // Достаточная capacity
    int v1 = 1, v2 = 2, v3 = 99;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.insert(cb.cbegin() + 1, v3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 99, 2));
}

TYPED_TEST(CircularBufferIntTest, insertFromListIterators) {
    TypeParam cb{1, 6};
    std::list<int> lst{2, 3, 4, 5};
    cb.insert(cb.cbegin() + 1, lst.begin(), lst.end());
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TYPED_TEST(CircularBufferIntTest, insertMultipleAtBeginRepeatedly) {
    TypeParam cb(100); // Достаточная capacity
    for (int i = 0; i < 20; ++i) {
        cb.insert(cb.cbegin(), as_lvalue(i));
    }
    EXPECT_EQ(cb.size(), 20u);
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(cb[static_cast<size_t>(i)], 19 - i);
    }
}

// ==================== Erase Tests ====================

TYPED_TEST(CircularBufferIntTest, eraseSingle) {
    TypeParam cb{1, 2, 3, 4, 5};
    auto it = cb.erase(cb.cbegin() + 2);
    EXPECT_EQ(*it, 4);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, eraseRange) {
    TypeParam cb{1, 2, 3, 4, 5};
    auto it = cb.erase(cb.cbegin() + 1, cb.cbegin() + 4);
    EXPECT_EQ(*it, 5);
    ASSERT_THAT(cb, testing::ElementsAre(1, 5));
}

TYPED_TEST(CircularBufferIntTest, eraseEmptyRange) {
    TypeParam cb{1, 2, 3};
    auto it = cb.erase(cb.cbegin() + 1, cb.cbegin() + 1);
    EXPECT_EQ(cb.size(), 3u);
    EXPECT_EQ(*it, 2);
}

TYPED_TEST(CircularBufferIntTest, eraseFirst) {
    TypeParam cb{1, 2, 3};
    cb.erase(cb.cbegin());
    ASSERT_THAT(cb, testing::ElementsAre(2, 3));
}

TYPED_TEST(CircularBufferIntTest, eraseLast) {
    TypeParam cb{1, 2, 3};
    cb.erase(cb.cend() - 1);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2));
}

TYPED_TEST(CircularBufferIntTest, eraseAll) {
    TypeParam cb{1, 2, 3};
    cb.erase(cb.cbegin(), cb.cend());
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, eraseUntilEmpty) {
    TypeParam cb{1, 2, 3, 4, 5};
    while (!cb.empty()) {
        cb.erase(cb.cbegin());
    }
    EXPECT_TRUE(cb.empty());
}

// ==================== Clear Tests ====================

TYPED_TEST(CircularBufferIntTest, clearNonEmpty) {
    TypeParam cb{1, 2, 3};
    cb.clear();
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, clearEmpty) {
    TypeParam cb;
    cb.clear();
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, clearPreservesCapacity) {
    TypeParam cb{1, 2, 3, 4, 5};
    size_t cap = cb.capacity();
    cb.clear();
    EXPECT_EQ(cb.capacity(), cap);
}

// ==================== Swap Tests ====================

TYPED_TEST(CircularBufferIntTest, memberSwap) {
    TypeParam a{1, 2, 3};
    TypeParam b{4, 5};
    a.swap(b);
    ASSERT_THAT(a, testing::ElementsAre(4, 5));
    ASSERT_THAT(b, testing::ElementsAre(1, 2, 3));
}

TYPED_TEST(CircularBufferIntTest, freeSwap) {
    TypeParam a{10};
    TypeParam b{20, 30};
    swap(a, b);
    ASSERT_THAT(a, testing::ElementsAre(20, 30));
    ASSERT_THAT(b, testing::ElementsAre(10));
}

TYPED_TEST(CircularBufferIntTest, swapWithEmpty) {
    TypeParam a{1, 2, 3};
    TypeParam b;
    a.swap(b);
    EXPECT_TRUE(a.empty());
    ASSERT_THAT(b, testing::ElementsAre(1, 2, 3));
}

TYPED_TEST(CircularBufferIntTest, swapBothEmpty) {
    TypeParam a;
    TypeParam b;
    a.swap(b);
    EXPECT_TRUE(a.empty());
    EXPECT_TRUE(b.empty());
}

// ==================== Iterator Tests ====================

TYPED_TEST(CircularBufferIntTest, beginEnd) {
    TypeParam cb{1, 2, 3};
    std::vector<int> v(cb.begin(), cb.end());
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TYPED_TEST(CircularBufferIntTest, constBeginEnd) {
    const TypeParam cb{4, 5, 6};
    std::vector<int> v(cb.begin(), cb.end());
    EXPECT_EQ(v, (std::vector<int>{4, 5, 6}));
}

TYPED_TEST(CircularBufferIntTest, cbeginCend) {
    TypeParam cb{7, 8};
    std::vector<int> v(cb.cbegin(), cb.cend());
    EXPECT_EQ(v, (std::vector<int>{7, 8}));
}

TYPED_TEST(CircularBufferIntTest, reverseIterators) {
    TypeParam cb{1, 2, 3, 4};
    std::vector<int> v(cb.rbegin(), cb.rend());
    EXPECT_EQ(v, (std::vector<int>{4, 3, 2, 1}));
}

TYPED_TEST(CircularBufferIntTest, constReverseIterators) {
    const TypeParam cb{10, 20, 30};
    std::vector<int> v(cb.rbegin(), cb.rend());
    EXPECT_EQ(v, (std::vector<int>{30, 20, 10}));
}

TYPED_TEST(CircularBufferIntTest, crbeginCrend) {
    TypeParam cb{1, 2, 3};
    std::vector<int> v(cb.crbegin(), cb.crend());
    EXPECT_EQ(v, (std::vector<int>{3, 2, 1}));
}

TYPED_TEST(CircularBufferIntTest, iteratorRandomAccess) {
    TypeParam cb{10, 20, 30, 40, 50};
    auto it = cb.begin();
    EXPECT_EQ(it[0], 10);
    EXPECT_EQ(it[2], 30);
    EXPECT_EQ(it[4], 50);
    it += 3;
    EXPECT_EQ(*it, 40);
    it -= 2;
    EXPECT_EQ(*it, 20);
}

TYPED_TEST(CircularBufferIntTest, iteratorArithmeticDifference) {
    TypeParam cb{1, 2, 3, 4, 5};
    auto b = cb.begin();
    auto e = cb.end();
    EXPECT_EQ(e - b, 5);
    EXPECT_EQ(b + 5, e);
    EXPECT_EQ(5 + b, e);
    EXPECT_EQ(e - 5, b);
}

TYPED_TEST(CircularBufferIntTest, iteratorComparisons) {
    TypeParam cb{1, 2, 3};
    auto b = cb.begin();
    auto e = cb.end();
    EXPECT_TRUE(b < e);
    EXPECT_TRUE(b <= e);
    EXPECT_TRUE(e > b);
    EXPECT_TRUE(e >= b);
    EXPECT_TRUE(b <= b);
    EXPECT_TRUE(b >= b);
    EXPECT_FALSE(b > e);
}

TYPED_TEST(CircularBufferIntTest, iteratorPrePostIncrDecr) {
    TypeParam cb{1, 2, 3};
    auto it = cb.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 3);
    EXPECT_EQ(*it--, 3);
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*--it, 1);
}

TYPED_TEST(CircularBufferIntTest, emptyIteratorRange) {
    TypeParam cb;
    EXPECT_EQ(cb.begin(), cb.end());
    EXPECT_EQ(cb.cbegin(), cb.cend());
    EXPECT_EQ(cb.rbegin(), cb.rend());
}

TYPED_TEST(CircularBufferIntTest, constConversion) {
    TypeParam cb{1, 2, 3};
    typename TypeParam::iterator it = cb.begin();
    typename TypeParam::const_iterator cit = it;
    EXPECT_EQ(*cit, 1);
}

TYPED_TEST(CircularBufferStringTest, iteratorArrow) {
    TypeParam cb(10); // Нужна capacity
    std::string s1 = "hello", s2 = "world";
    cb.push_back(s1);
    cb.push_back(s2);
    auto it = cb.begin();
    EXPECT_EQ(it->size(), 5u);
    ++it;
    EXPECT_EQ(it->size(), 5u);
}

TYPED_TEST(CircularBufferIntTest, iteratorDefaultConstructed) {
    typename TypeParam::iterator it1;
    typename TypeParam::iterator it2;
    EXPECT_EQ(it1, it2);
}

// ==================== Comparison Tests ====================

TYPED_TEST(CircularBufferIntTest, equalBuffers) {
    TypeParam a{1, 2, 3};
    TypeParam b{1, 2, 3};
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TYPED_TEST(CircularBufferIntTest, notEqualBuffers) {
    TypeParam a{1, 2, 3};
    TypeParam b{1, 2, 4};
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TYPED_TEST(CircularBufferIntTest, differentSizes) {
    TypeParam a{1, 2};
    TypeParam b{1, 2, 3};
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TYPED_TEST(CircularBufferIntTest, lessThan) {
    TypeParam a{1, 2, 3};
    TypeParam b{1, 2, 4};
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TYPED_TEST(CircularBufferIntTest, lessThanPrefix) {
    TypeParam a{1, 2};
    TypeParam b{1, 2, 3};
    EXPECT_TRUE(a < b);
}

TYPED_TEST(CircularBufferIntTest, greaterThan) {
    TypeParam a{1, 3};
    TypeParam b{1, 2};
    EXPECT_TRUE(a > b);
}

TYPED_TEST(CircularBufferIntTest, lessEqualGreaterEqual) {
    TypeParam a{1, 2, 3};
    TypeParam b{1, 2, 3};
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a >= b);
}

TYPED_TEST(CircularBufferIntTest, emptyBuffersEqual) {
    TypeParam a;
    TypeParam b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
}

// ==================== Non-extendable Overwrite Tests ====================

TEST(NonExtendable, circularOverwritePushBack) {
    circular_buffer<int, false> cb(5);
    for (int i = 0; i < 5; ++i) {
        cb.push_back(as_lvalue(i));
    }
    EXPECT_TRUE(cb.full());
    int v5 = 5, v6 = 6;
    cb.push_back(v5);
    cb.push_back(v6);
    EXPECT_EQ(cb.size(), 5u);
    EXPECT_EQ(cb.front(), 2);
    EXPECT_EQ(cb.back(), 6);
}

TEST(NonExtendable, circularOverwritePushFront) {
    circular_buffer<int, false> cb(3);
    int v1 = 1, v2 = 2, v3 = 3, v0 = 0;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_front(v0);
    EXPECT_EQ(cb.size(), 3u);
    EXPECT_EQ(cb.front(), 0);
    EXPECT_EQ(cb.back(), 2);
}

TEST(NonExtendable, fullCheck) {
    circular_buffer<int, false> cb(3);
    EXPECT_FALSE(cb.full());
    int v1 = 1, v2 = 2, v3 = 3;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    EXPECT_TRUE(cb.full());
}

TEST(NonExtendable, manyOverwrites) {
    circular_buffer<int, false> cb(3);
    for (int i = 0; i < 100; ++i) {
        cb.push_back(as_lvalue(i));
    }
    EXPECT_EQ(cb.size(), 3u);
    ASSERT_THAT(cb, testing::ElementsAre(97, 98, 99));
}

// ==================== Wrap-around Tests ====================

TYPED_TEST(CircularBufferIntTest, wraparoundPushPop) {
    TypeParam cb(4);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(v4);
    cb.push_back(v5);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, wraparoundIteration) {
    TypeParam cb(4);
    int v10 = 10, v20 = 20, v30 = 30, v40 = 40, v50 = 50, v60 = 60;
    cb.push_back(v10);
    cb.push_back(v20);
    cb.push_back(v30);
    cb.push_back(v40);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(v50);
    cb.push_back(v60);
    std::vector<int> result(cb.begin(), cb.end());
    EXPECT_EQ(result, (std::vector<int>{30, 40, 50, 60}));
}

TYPED_TEST(CircularBufferIntTest, reserveLinearizesWrapped) {
    TypeParam cb(4);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_back(v4);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(v5);
    cb.push_back(v6);
    cb.reserve(10);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5, 6));
}

TYPED_TEST(CircularBufferIntTest, eraseAfterWraparound) {
    TypeParam cb(4);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_back(v4);
    cb.pop_front();
    cb.push_back(v5);
    cb.erase(cb.cbegin() + 1);
    ASSERT_THAT(cb, testing::ElementsAre(2, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, insertAfterWraparound) {
    TypeParam cb(10); // Достаточная capacity для insert
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v99 = 99;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.pop_front();
    cb.push_back(v4);
    cb.insert(cb.cbegin() + 1, v99);
    ASSERT_THAT(cb, testing::ElementsAre(2, 99, 3, 4));
}

// ==================== String Tests ====================

TYPED_TEST(CircularBufferStringTest, pushPopString) {
    TypeParam cb(10); // Нужна capacity
    std::string s1 = "hello", s2 = "world", s3 = "start";
    cb.push_back(s1);
    cb.push_back(s2);
    cb.push_front(s3);
    EXPECT_EQ(cb.front(), "start");
    EXPECT_EQ(cb.back(), "world");
    cb.pop_back();
    EXPECT_EQ(cb.back(), "hello");
}

TYPED_TEST(CircularBufferStringTest, insertEraseString) {
    TypeParam cb(10); // Нужна capacity
    std::string s1 = "alpha", s2 = "gamma", s3 = "beta";
    cb.push_back(s1);
    cb.push_back(s2);
    cb.insert(cb.cbegin() + 1, s3);
    ASSERT_THAT(cb, testing::ElementsAre("alpha", "beta", "gamma"));
    cb.erase(cb.cbegin() + 1);
    ASSERT_THAT(cb, testing::ElementsAre("alpha", "gamma"));
}

TYPED_TEST(CircularBufferStringTest, assignString) {
    TypeParam cb(10); // Нужна capacity
    std::vector<std::string> v{"one", "two", "three"};
    cb.assign(v.begin(), v.end());
    ASSERT_THAT(cb, testing::ElementsAre("one", "two", "three"));
}

TYPED_TEST(CircularBufferStringTest, clearString) {
    TypeParam cb(10); // Нужна capacity
    std::string s1 = "a", s2 = "b", s3 = "c";
    cb.push_back(s1);
    cb.push_back(s2);
    cb.push_back(s3);
    cb.clear();
    EXPECT_TRUE(cb.empty());
}

// ==================== STL Algorithm Compatibility ====================

TYPED_TEST(CircularBufferIntTest, stdSort) {
    TypeParam cb{5, 3, 1, 4, 2};
    std::sort(cb.begin(), cb.end());
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTest, stdFind) {
    TypeParam cb{10, 20, 30, 40};
    auto it = std::find(cb.begin(), cb.end(), 30);
    EXPECT_NE(it, cb.end());
    EXPECT_EQ(*it, 30);
    auto it2 = std::find(cb.begin(), cb.end(), 99);
    EXPECT_EQ(it2, cb.end());
}

TYPED_TEST(CircularBufferIntTest, stdAccumulate) {
    TypeParam cb{1, 2, 3, 4, 5};
    int sum = std::accumulate(cb.begin(), cb.end(), 0);
    EXPECT_EQ(sum, 15);
}

TYPED_TEST(CircularBufferIntTest, rangeBasedFor) {
    TypeParam cb{1, 2, 3};
    int sum = 0;
    for (auto& x : cb) sum += x;
    EXPECT_EQ(sum, 6);
}

TYPED_TEST(CircularBufferIntTest, rangeBasedForConst) {
    const TypeParam cb{10, 20, 30};
    int sum = 0;
    for (const auto& x : cb) sum += x;
    EXPECT_EQ(sum, 60);
}

TYPED_TEST(CircularBufferIntTest, stdCopyToVector) {
    TypeParam cb{1, 2, 3, 4};
    std::vector<int> v;
    std::copy(cb.begin(), cb.end(), std::back_inserter(v));
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4}));
}

TYPED_TEST(CircularBufferIntTest, stdTransform) {
    TypeParam cb{1, 2, 3};
    std::transform(cb.begin(), cb.end(), cb.begin(), [](int x) { return x * 2; });
    ASSERT_THAT(cb, testing::ElementsAre(2, 4, 6));
}

TYPED_TEST(CircularBufferIntTest, stdReverse) {
    TypeParam cb{1, 2, 3, 4, 5};
    std::reverse(cb.begin(), cb.end());
    ASSERT_THAT(cb, testing::ElementsAre(5, 4, 3, 2, 1));
}

TYPED_TEST(CircularBufferIntTest, stdFillN) {
    TypeParam cb{0, 0, 0, 0, 0};
    std::fill_n(cb.begin(), 3, 42);
    ASSERT_THAT(cb, testing::ElementsAre(42, 42, 42, 0, 0));
}

TYPED_TEST(CircularBufferIntTest, stdCount) {
    TypeParam cb{1, 2, 2, 3, 2};
    auto cnt = std::count(cb.begin(), cb.end(), 2);
    EXPECT_EQ(cnt, 3);
}

TYPED_TEST(CircularBufferIntTest, stdMinMaxElement) {
    TypeParam cb{5, 1, 3, 2, 4};
    auto minIt = std::min_element(cb.begin(), cb.end());
    auto maxIt = std::max_element(cb.begin(), cb.end());
    EXPECT_EQ(*minIt, 1);
    EXPECT_EQ(*maxIt, 5);
}

TYPED_TEST(CircularBufferIntTest, stdPartialSort) {
    TypeParam cb{5, 3, 1, 4, 2};
    std::partial_sort(cb.begin(), cb.begin() + 3, cb.end());
    EXPECT_EQ(cb[0], 1);
    EXPECT_EQ(cb[1], 2);
    EXPECT_EQ(cb[2], 3);
}

TYPED_TEST(CircularBufferIntTest, stdNthElement) {
    TypeParam cb{5, 3, 1, 4, 2};
    std::nth_element(cb.begin(), cb.begin() + 2, cb.end());
    EXPECT_EQ(cb[2], 3);
}

// ==================== Edge Cases ====================

TYPED_TEST(CircularBufferIntTest, singleElement) {
    TypeParam cb(10); // Нужна capacity
    int v = 42;
    cb.push_back(v);
    EXPECT_EQ(cb.front(), 42);
    EXPECT_EQ(cb.back(), 42);
    cb.pop_back();
    EXPECT_TRUE(cb.empty());
}

TYPED_TEST(CircularBufferIntTest, largeBuffer) {
    TypeParam cb(10000); // Достаточная capacity
    const int N = 10000;
    for (int i = 0; i < N; ++i) {
        cb.push_back(as_lvalue(i));
    }
    EXPECT_EQ(cb.size(), static_cast<size_t>(N));
    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(cb[static_cast<size_t>(i)], i);
    }
}

TYPED_TEST(CircularBufferIntTest, alternatingPushFrontBackMixed) {
    TypeParam cb(100); // Достаточная capacity
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            cb.push_back(as_lvalue(i));
        } else {
            cb.push_front(as_lvalue(i));
        }
    }
    EXPECT_EQ(cb.size(), 100u);
}

TYPED_TEST(CircularBufferIntTest, pushPopPushSequence) {
    TypeParam cb(100); // Достаточная capacity
    for (int i = 0; i < 50; ++i) {
        cb.push_back(as_lvalue(i));
    }
    for (int i = 0; i < 25; ++i) {
        cb.pop_front();
    }
    for (int i = 50; i < 100; ++i) {
        cb.push_back(as_lvalue(i));
    }
    EXPECT_EQ(cb.size(), 75u);
    EXPECT_EQ(cb.front(), 25);
    EXPECT_EQ(cb.back(), 99);
}

TYPED_TEST(CircularBufferIntTest, clearAndReuse) {
    TypeParam cb{1, 2, 3};
    cb.clear();
    int v10 = 10, v20 = 20;
    cb.push_back(v10);
    cb.push_back(v20);
    ASSERT_THAT(cb, testing::ElementsAre(10, 20));
}

TYPED_TEST(CircularBufferIntTest, resizeAfterWraparound) {
    TypeParam cb(4);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_back(v4);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(v5);
    cb.push_back(v6);
    cb.resize(2);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4));
}

TYPED_TEST(CircularBufferIntTest, assignAfterWraparound) {
    TypeParam cb(10); // Достаточная capacity
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    cb.push_back(v1);
    cb.push_back(v2);
    cb.push_back(v3);
    cb.push_back(v4);
    cb.pop_front();
    cb.push_back(v5);
    cb.assign({10, 20, 30});
    ASSERT_THAT(cb, testing::ElementsAre(10, 20, 30));
}

TYPED_TEST(CircularBufferIntTest, copyAssignmentWrapped) {
    TypeParam a(4);
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    a.push_back(v1);
    a.push_back(v2);
    a.push_back(v3);
    a.push_back(v4);
    a.pop_front();
    a.push_back(v5);
    TypeParam b;
    b = a;
    ASSERT_THAT(b, testing::ElementsAre(2, 3, 4, 5));
}

// ==================== Allocator Tests ====================

TYPED_TEST(CircularBufferIntTest, getAllocator) {
    TypeParam cb;
    auto a = cb.get_allocator();
    (void)a;
}

// ==================== Type Traits ====================

TEST(TypeTraits, typeAliases) {
    using CB = circular_buffer<int>;
    static_assert(std::is_same_v<CB::value_type, int>);
    static_assert(std::is_same_v<CB::reference, int&>);
    static_assert(std::is_same_v<CB::const_reference, const int&>);
    static_assert(std::is_same_v<CB::size_type, std::size_t>);
    static_assert(std::is_same_v<CB::difference_type, std::ptrdiff_t>);
    static_assert(std::is_same_v<CB::allocator_type, std::allocator<int>>);
}

TEST(TypeTraits, iteratorTraits) {
    using It = circular_buffer<int>::iterator;
    static_assert(std::is_same_v<
        std::iterator_traits<It>::iterator_category,
        std::random_access_iterator_tag>);
    static_assert(std::is_same_v<std::iterator_traits<It>::value_type, int>);
    static_assert(std::is_same_v<std::iterator_traits<It>::difference_type, std::ptrdiff_t>);
}

TEST(TypeTraits, constIteratorTraits) {
    using CIt = circular_buffer<int>::const_iterator;
    static_assert(std::is_same_v<
        std::iterator_traits<CIt>::iterator_category,
        std::random_access_iterator_tag>);
    static_assert(std::is_same_v<std::iterator_traits<CIt>::reference, const int&>);
}