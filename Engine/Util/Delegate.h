#pragma once
#include <vector>
#include <functional>
#include <algorithm>

// 핸들 타입
using DelegateHandle = unsigned long long;

// 멤버 함수를 델리게이트 콜백으로 변환 (인자 전달)
#define BIND_ARGS(Object, Function) \
    [Object](auto&&... args) { (Object)->Function(std::forward<decltype(args)>(args)...); }

// 멤버 함수를 델리게이트 콜백으로 변환 (인자 없음)
#define BIND(Object, Function) \
    [Object]() { (Object)->Function(); }

// 멀티캐스트 델리게이트
template<typename... Args>
class MulticastDelegate
{
public:
    using EventFunc = std::function<void(Args...)>;

private:
    // 내부 저장용 구조체
    struct Listener
    {
        DelegateHandle handle;
        EventFunc       func;
    };

    std::vector<Listener> listeners;
    DelegateHandle       idCounter = 0; // 핸들 발급용 generator

public:
    MulticastDelegate() {}
    ~MulticastDelegate() { Clear(); }

    // 콜백 등록
    DelegateHandle Add(EventFunc func)
    {
        DelegateHandle newHandle = ++idCounter;
        listeners.push_back({ newHandle, func });
        return newHandle;
    }

    // 콜백 제거
    void Remove(DelegateHandle& handle)
    {
        if (handle == 0) return;

        auto iter = std::remove_if(listeners.begin(), listeners.end(),
            [handle](const Listener& listener) 
            {
                return listener.handle == handle;
            });

        if (iter != listeners.end())
        {
            listeners.erase(iter, listeners.end());
            handle = 0; // 핸들 무효화
        }
    }

    // 모든 콜백 제거
    void Clear()
    {
        listeners.clear();
    }

    // 콜백 실행
    void Broadcast(Args... args)
    {
        if (listeners.empty()) 
            return;

        for (int i = listeners.size() - 1; i >= 0; --i)
        {
            const auto& listener = listeners[i];
            if (listener.func)
            {
                listener.func(std::forward<Args>(args)...);
            }
        }
    }
    
    void operator()(Args... args)
    {
        Broadcast(std::forward<Args>(args)...);
    }

    // 바인딩 여부 확인
    bool IsBound() const
    {
        return !listeners.empty();
    }
};

// 단일 델리게이트
template<typename... Args>
class Delegate
{
public:
    using EventFunc = std::function<void(Args...)>;

private:
    EventFunc callback;

public:
    Delegate() : callback(nullptr) {}
    ~Delegate() { Clear(); }

    // 콜백 바인딩
    void Bind(EventFunc func)
    {
        callback = func;
    }
    
    // 콜백 제거
    void Clear()
    {
        callback = nullptr;
    }

    // 콜백 실행
    void Invoke(Args... args)
    {
        if (callback)
        {
            callback(std::forward<Args>(args)...);
        }
    }

    void operator()(Args... args)
    {
        Invoke(std::forward<Args>(args)...);
    }

    // 바인딩 여부 확인
    bool IsBound() const
    {
        return callback != nullptr;
    }
};