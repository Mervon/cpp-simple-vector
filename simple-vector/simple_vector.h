#pragma once

#include <algorithm>
#include <utility>
#include <cassert>
#include <initializer_list>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) : capacity_(capacity_to_reserve){};
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : SimpleVector(size, Type()) {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : array_(size)  {
        capacity_ = size;
        size_ = size;
        std::fill(begin(), end(), Type(value));
    }
    
    SimpleVector(size_t size, Type&& value) : array_(new Type[size]{}) {
		array_[0] = std::move(value);
        size_ = size;
        capacity_ = size;
	}  

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector(ReserveProxyObj objc) {
        Reserve(objc.capacity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return 0 == size_;
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
            using namespace std::string_literals;
            throw std::out_of_range("index >= size"s);
        } else {
            return array_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            using namespace std::string_literals;
            throw std::out_of_range("index >= size"s);
        } else {
            return array_[index];
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (size_ < new_size) {
            if (new_size <= capacity_)
            {
                auto first = array_.Get() + size_;
                auto last = array_.Get() + new_size;
                assert(first < last);
                while (first < last) {
                    *first = std::move(Type());
                    first++;
                }
            }
            else
            {
                size_t new_capacity = std::max(new_size, capacity_ * 2);
                ArrayPtr<Type> tmp_array{new_capacity};
                std::copy(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), tmp_array.Get());      
                for (auto it = tmp_array.Get() + size_; it != tmp_array.Get() + new_size; ++ it) {
                    *it = std::move(Type{});
                }
                array_.swap(tmp_array);
                capacity_ = new_capacity;
            }
        }
        size_ = new_size;
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
    
    SimpleVector(const SimpleVector& other) {
        
        SimpleVector<Type> tmp(other.GetSize());

        std::copy(other.begin(), other.end(), tmp.begin());
        
        swap(tmp);
    }

    SimpleVector(SimpleVector&& other) {  
    Reserve(other.GetSize()); 
    size_ = other.GetSize();
    
    std::move_backward(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), end()); //по факту без временного вектора мы не обошлись, он создался в методе Reserve() в 181 строке, так что здесь идентичное кол-во созданий нового вектора, да и std::move_backward всё что делает так это проходится по итераторам от начала и до конца в точно таком же цикле что был раньше, и еденственное что мы получили с этого нововведения - это читабельность кода (было 3 строки, а стало 1), или быть может я что то не правильно понимаю?

    while (other.GetSize() != 0) {
        other.PopBack();
    }
}

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy{rhs};
            swap(rhs_copy);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            size_t new_capacity = std::max(static_cast<size_t>(1), capacity_ * 2);
            ArrayPtr<Type> tmp_array{new_capacity};
            std::copy(this->begin(), this->end(), tmp_array.Get());
            tmp_array[size_] = item;
            array_.swap(tmp_array);
            capacity_ = new_capacity;
            ++size_;
        } else {
            array_[size_++] = item;
        }
    }
    
    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            size_t new_capacity = std::max(static_cast<size_t>(1), capacity_ * 2);
            ArrayPtr<Type> tmp_array{new_capacity};
            std::move(std::make_move_iterator(this->begin()), std::make_move_iterator(this->end()), tmp_array.Get());
            tmp_array[size_] = std::move(item);
            array_.swap(tmp_array);
            capacity_ = new_capacity;
            ++size_;
        } else {
            array_[size_++] = std::move(item);
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_size_t = std::distance(begin(), Iterator(pos));
        assert(pos_size_t <= size_);
        auto pos_size_t_end = std::distance(begin(), end());
        if (size_ == capacity_) {
            if (size_ == 0) {
                capacity_ = pos_size_t + 1;
                ArrayPtr<Type> buff(capacity_);
                Type it{};
                std::fill(buff.Get(), buff.Get() + capacity_, it);
                buff[pos_size_t] = value;
                array_.swap(buff);
            } else {
                capacity_ = std::max(pos_size_t, capacity_ * 2);
                ArrayPtr<Type> buff(capacity_);
                std::copy(begin(), Iterator(pos), buff.Get());
                buff[pos_size_t] = value;
                std::copy_backward(Iterator(pos), end(), buff.Get() + pos_size_t_end + 1);
                array_.swap(buff);
            }
        } else {
            std::copy_backward(Iterator(pos), end(), end() + 1);
            array_[pos_size_t] = value;
        }
        ++size_;
        return &array_[pos_size_t];
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_size_t = std::distance(begin(), Iterator(pos));
        assert(pos_size_t <= size_);
        auto pos_size_t_end = std::distance(begin(), end());
        if (size_ == capacity_) {
            if (size_ == 0) {
                capacity_ = pos_size_t + 1;
                ArrayPtr<Type> buff(capacity_);
                auto first = buff.Get() + size_;
                auto last = buff.Get() + capacity_;
                assert(first < last);
                while (first < last) {
                    *first = std::move(Type());
                    first++;
                }
                buff[pos_size_t] = std::move(value);
                array_.swap(buff);
            } else {
                capacity_ = std::max(pos_size_t, capacity_ * 2);
                ArrayPtr<Type> buff(capacity_);
                std::copy(std::make_move_iterator(begin()), std::make_move_iterator(Iterator(pos)), buff.Get());
                buff[pos_size_t] = std::move(value);
                std::move_backward(std::make_move_iterator(Iterator(pos)), std::make_move_iterator(end()), buff.Get() + pos_size_t_end + 1);
                array_.swap(buff);
            }
        } else {
            std::move_backward(std::make_move_iterator(Iterator(pos)), std::make_move_iterator(end()), end() + 1);
            array_[pos_size_t] = std::move(value);
        }
        ++size_;
        return &array_[pos_size_t];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t pos_size_t = pos - this->begin();
        assert(pos_size_t <= size_);
        size_t end = this->end() - this->begin();
        for (; (pos_size_t + 1) != end; ++pos_size_t) {
            array_[pos_size_t] = std::move(array_[pos_size_t + 1]);
        }
        --size_;
        return &array_[pos - this->begin()];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        other.array_.swap(array_);
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp_array{new_capacity};
            std::move(std::make_move_iterator(array_.Get()), std::make_move_iterator(array_.Get() + size_), tmp_array.Get());
            array_.swap(tmp_array);
            
            capacity_ = new_capacity;
        }
    }

private: 
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return  ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
void swap(SimpleVector<Type>& lhs, SimpleVector<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return (std::equal(lhs.begin(), lhs.end(), rhs.begin()));
    }
    return false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs==rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
} 