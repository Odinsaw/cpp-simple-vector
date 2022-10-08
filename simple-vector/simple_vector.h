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

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> temp(other.GetSize());
        temp.swap(array_);
        std::copy(other.begin(), other.end(), begin());
        size_ = other.size_;
        capacity_ = other.size_;
    }

    SimpleVector(SimpleVector&& other) {
        ArrayPtr<Type> temp(other.GetSize());
        temp.swap(array_);
        std::move(other.begin(), other.end(), begin());
        size_ = other.size_;
        capacity_ = other.size_;
        other.size_ = 0;
    }

    SimpleVector(const ReserveProxyObj& input) {
        capacity_ = input.capacity_;
        ArrayPtr<Type> temp(size_);
        temp.swap(array_);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        :capacity_(size), size_(size)
    {
        ArrayPtr<Type> temp(size_);
        temp.swap(array_);
        for (auto it = begin(); it != end(); ++it) {
            *it = std::move(Type());
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        :capacity_(size), size_(size)
    {
        ArrayPtr<Type> temp(size_);
        temp.swap(array_);
        std::fill(begin(), end(), value);
    }

    SimpleVector(size_t size, Type&& value)
        :capacity_(size), size_(size)
    {
        ArrayPtr<Type> temp(size_);
        temp.swap(array_);
        //std::fill(begin(), end(), value);
        for (auto it = begin(); it != end(); ++it) {
            *it = std::move(Type());
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
    {
        size_ = init.size();
        capacity_ = init.size();
        ArrayPtr<Type> temp(size_);
        temp.swap(array_);
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
        if (this == &rhs) {
            return *this;
        }
        SimpleVector temp(rhs);
        swap(temp);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    void PushBack(const Type& value) {
        auto old_size = size_;
        if (capacity_ == 0) {
            Resize(1);
        }
        else if (size_ >= capacity_) {
            Resize(2 * capacity_);
        }
        size_ = old_size;
        (*this)[size_] = value;
        ++size_;
    }

    void PushBack(Type&& value) {
        auto old_size = size_;
        if (capacity_ == 0) {
            Reserve(1);
        }
        else if (size_ >= capacity_) {
            Reserve(2 * capacity_);
        }
        size_ = old_size;
        (*this)[size_] = std::move(value);
        ++size_;
    }

    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
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
            try {
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
            catch (...) {
                throw;
            }
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
            try {
                //SimpleVector<Type> temp(std::max(static_cast<size_t>(1u), 2 * size_));
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
            catch (...) {
                throw;
            }
        }
    }

    void swap(SimpleVector& other) noexcept {
        array_.swap(other.array_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    void swap(SimpleVector&& other) noexcept {
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
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
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
        if (new_size > capacity_) {
            try {
                SimpleVector<Type> temp(new_size);
                std::move(begin(), end(), temp.begin());
                array_.swap(temp.array_);
                capacity_ = new_size;
                size_ = new_size;
            }
            catch (...) {
                throw;
            }
        }
        else if (new_size < size_) {
            size_ = new_size;
        }
        else {
            //std::fill(begin()+size_, begin() + capacity_, Type());
            for (auto it = begin(); it != end(); ++it) {
                //Type temp;
                *it = std::move(Type());
            }
            size_ = new_size;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            try {
                ArrayPtr<Type> temp(new_capacity);
                std::move(begin(), end(), temp.Get());
                temp.swap(array_);
                capacity_ = new_capacity;
            }
            catch (...) {
                throw;
            }
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
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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