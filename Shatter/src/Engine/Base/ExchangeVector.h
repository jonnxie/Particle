//
// Created by jonnxie on 2022/4/26.
//

#ifndef MAIN_EXCHANGEVECTOR_H
#define MAIN_EXCHANGEVECTOR_H

#include "Engine/Item/shatter_macro.h"
#include <vector>
#include <memory>
#include <algorithm>

template<class T>
class ExchangeVector {
public:
    ExchangeVector() {
        preVec = std::make_unique<std::vector<T>>();
        nowVec = std::make_unique<std::vector<T>>();
    };
    explicit ExchangeVector(size_t _count) {
        preVec = std::make_unique<std::vector<T>>();
        nowVec = std::make_unique<std::vector<T>>();
        preVec.resize(_count);
        nowVec.resize(_count);
    }
    DefineUnCopy(ExchangeVector);
public:
    T& operator[](size_t _index) {
        return (*preVec.get())[_index];
    }

    std::vector<T>& operator()() {
        return *preVec.get();
    }

    void push_back(T& _element) {
        nowVec->push_back(_element);
        changed = true;
    }

    void erase(T& _element) {
        auto it = std::find(nowVec->begin(), nowVec->end(), _element);
        if (it != nowVec->end()) {
            nowVec->erase(it);
            changed = true;
        }
    }

    size_t size() {
        return preVec->size();
    }

    bool empty() {
        return preVec->empty();
    }

    void clear() {
        nowVec->clear();
        changed = true;
    }
public:
    void flush() {
        if (changed) {
            std::swap(preVec, nowVec);
            nowVec->resize(preVec->size());
            memcpy(nowVec->data(), preVec->data(), sizeof(T) * nowVec->size());
            changed = false;
        }
    }

private:
    bool changed = false;
    std::unique_ptr<std::vector<T>> preVec{nullptr};
    std::unique_ptr<std::vector<T>> nowVec{nullptr};
};


#endif //MAIN_EXCHANGEVECTOR_H
