#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>

using namespace std::literals;

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve)
        :capacity_(capacity_to_reserve)
    {

    }
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(const SimpleVector& other)
        :size_(other.size_), capacity_(other.size_), array_(other.size_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)
        :size_(other.size_), capacity_(other.size_), array_(other.size_)
    {
        swap(other);
        other.Clear(); //тест TestNamedMoveConstructor() требует, чтобы вектор был пустым
    }

    SimpleVector(const ReserveProxyObj& input)
        :capacity_(input.capacity_)
    {
        Reserve(capacity_);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        :capacity_(size), size_(size), array_(size)
    {
        for (auto it = begin(); it != end(); ++it) {
            *it = Type();
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        :capacity_(size), size_(size), array_(size)
    {
        std::fill(begin(), end(), value);
    }

    //SimpleVector(size_t size, Type&& value)
    //    :capacity_(size), size_(size),array_(size)
    //{
    //    for (auto it = begin(); it != end(); ++it) {
    //        *it = Type(value);
    //    }
    //}

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        :size_(init.size()), capacity_(init.size()), array_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector temp(rhs);
        swap(temp);
        return *this;
    }

    SimpleVector operator=(SimpleVector&& rhs) {
        if (this != &rhs)
        {
            swap(rhs);
            //rhs.Clear();
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    void PushBack(const Type& value) {
        if (capacity_ == 0) {
            Reserve(1);
        }
        else if (size_ >= capacity_) {
            Reserve(2 * capacity_);
        }
        (*this).array_[size_] = value;
        ++size_;
    }

    void PushBack(Type&& value) {
        if (capacity_ == 0) {
            Reserve(1);
        }
        else if (size_ >= capacity_) {
            Reserve(2 * capacity_);
        }
        (*this).array_[size_] = std::move(value);
        ++size_;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
            --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos < cend());
        auto p = const_cast<Iterator>(pos);
        std::move(p + 1, end(), p);
        --size_;
        return p;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        auto p = const_cast<Iterator>(pos);
        if (size_ < capacity_) {
            std::copy_backward(p, end(), end() + 1);
            (*p) = value;
            ++size_;
            return p;
        }
        else {
                SimpleVector<Type> temp(std::max(static_cast<size_t>(1u), 2 * size_));
                auto s = temp.begin();
                if (p != begin()) {
                    s = std::copy(begin(), p, temp.begin());
                }
                *s = value;
                std::copy(p, end(), s + 1);
                array_.swap(temp.array_);
                capacity_ = 2 * std::max(static_cast<size_t>(1u), 2 * size_);
                ++size_;
                return s;
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        auto p = const_cast<Iterator>(pos);
        if (size_ < capacity_) {
            std::move_backward(p, end(), end() + 1);
            *p = std::move(value);
            ++size_;
            return p;
        }
        else {
                SimpleVector<Type> temp;
                temp.Reserve(std::max(static_cast<size_t>(1u), 2 * size_));
                auto s = temp.begin();
                if (p != begin()) {
                    s = std::move(begin(), p, temp.begin());
                }
                *s = std::move(value);
                std::move(p, end(), s + 1);
                array_.swap(temp.array_);
                capacity_ = 2 * std::max(static_cast<size_t>(1u), 2 * size_);
                ++size_;
                return s;
        }
    }

    void swap(SimpleVector& other) noexcept {
        array_.swap(other.array_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range!"s);
        }
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range!"s);
        }
        return array_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        }
        else {
            if (new_size > capacity_) {
                Reserve(new_size);
            }
            for (auto it = begin() + size_; it < begin() + capacity_; ++it) {
                *it = Type();
            }
            size_ = new_size;
        }

    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
                ArrayPtr<Type> temp(new_capacity);
                std::move(begin(), end(), temp.Get());
                temp.swap(array_);
                capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return array_.Get() + size_;
    }
private:
    size_t capacity_ = 0;
    size_t size_ = 0;
    ArrayPtr<Type> array_;
};

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;
}