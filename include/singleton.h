#pragma once

#include <mutex>
#include <memory>

template <typename T>
class Singleton {
public:
    // Deleted copy/move constructors and assignment operators
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    // Accessor for the singleton instance
    static T& Instance() {
        std::call_once(initFlag, []() {
            instance.reset(new T());
        });
        return *instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    static std::unique_ptr<T> instance;
    static std::once_flag initFlag;
};

// Static member definitions
template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initFlag;
