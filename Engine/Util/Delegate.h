#pragma once
#include <vector>
#include <functional>
#include <algorithm>

// 핸들 타입
using DelegateHandle = unsigned long long;

// 멤버 함수를 델리게이트 콜백으로 변환하는 매크로
// 사용 예: animator->Play(L"Idle", false, DELEGATE(this, OnAnimFinished));
#define DELEGATE(Object, Function) \
    [Object](auto&&... args) { Object->Function(std::forward<decltype(args)>(args)...); }

// 멀티캐스트 델리게이트 (여러 리스너 관리)
template<typename... Args>
class MulticastDelegate
{
public:
    // 실행할 함수 타입
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

    DelegateHandle Add(EventFunc func)
    {
        DelegateHandle newHandle = ++idCounter;
        listeners.push_back({ newHandle, func });
        return newHandle;
    }

    void Remove(DelegateHandle& handle)
    {
        if (handle == 0) return;

        auto iter = std::remove_if(listeners.begin(), listeners.end(),
            [handle](const Listener& listener) 
            {
                return listener.handle == handle;
            });

        // 실제로 삭제된 항목이 있을 때만 erase
        if (iter != listeners.end())
        {
            listeners.erase(iter, listeners.end());
            handle = 0; // 핸들 무효화
        }
    }

    void Clear()
    {
        listeners.clear();
    }

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

    // 연산자 오버로딩 (Broadcast)
    void operator()(Args... args)
    {
        Broadcast(std::forward<Args>(args)...);
    }

    bool IsBound() const
    {
        return !listeners.empty();
    }
};

// 단일 델리게이트 (하나의 콜백만 저장)
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

    // 콜백 해제
    void Unbind()
    {
        callback = nullptr;
    }

    // 콜백 제거 (Unbind와 동일)
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

    // 연산자 오버로딩 (Invoke)
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