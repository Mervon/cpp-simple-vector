#pragma once

#include <cassert>
#include <cstdlib>
#include <utility>
#include <iostream>

#include "simple_vector.h"

template <typename Type>
class ArrayPtr {

public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        if (size != 0) {
            raw_ptr_ = new Type[size];
        } else {
            raw_ptr_ = nullptr;
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) {
    }

    // Запрещаем копирование
    ArrayPtr(const ArrayPtr&) = delete;
    
    ArrayPtr(ArrayPtr&& other) {
        raw_ptr_ = move(other.Get());
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Запрещаем присваивание
    ArrayPtr& operator=(const ArrayPtr&) = delete;
    
    ArrayPtr& operator=(ArrayPtr&& other) {
        raw_ptr_ = move(other.Get());
    }


    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* copy_to_return = raw_ptr_;
        raw_ptr_ = nullptr;
        
        return copy_to_return;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {
        return raw_ptr_ == nullptr ? false : true; 
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        auto * temp = raw_ptr_;
        raw_ptr_ = other.Get();
        other.raw_ptr_ = temp;
    }
    
private:
    Type* raw_ptr_ = nullptr;
};