#pragma once
#include <vector>
#include <functional>
#include <algorithm>

// 핸들 타입
struct DelegateHandle
{
    DelegateHandle() : handle(0)
    {
    }

    DelegateHandle(unsigned long long id) : handle(id)
    {
    }

    bool IsValid() const { return handle > 0;}

    bool operator==(const DelegateHandle& other) const
    {
        return (handle == other.handle);
    }
private:
    unsigned long long handle;
};

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
        DelegateHandle  handle;
        EventFunc       func;
        bool            bPendingRemove = false;
    };

    std::vector<Listener> listeners;
    unsigned long long       idCounter = 0; // 핸들 발급용 generator

public:
    MulticastDelegate() {}
    ~MulticastDelegate() { Clear(); }

    // 콜백 등록
    DelegateHandle Add(EventFunc func)
    {
        DelegateHandle newHandle(++idCounter);
        listeners.push_back({ newHandle, func });
        return newHandle;
    }

    // 콜백 제거 (마킹 방식으로 Broadcast 중 안전하게 제거)
    void Remove(DelegateHandle& handle)
    {
        if (!handle.IsValid()) return;

        for (auto& listener : listeners)
        {
            if (listener.handle == handle)
            {
                listener.bPendingRemove = true;
                break;
            }
        }
        handle = 0; // 핸들 무효화
    }

    // 모든 콜백 제거
    void Clear()
    {
        listeners.clear();
    }

    // 콜백 실행 (마킹된 listener는 스킵, 실행 후 정리)
    void Broadcast(Args... args)
    {
        if (listeners.empty())
            return;

        for (auto& listener : listeners)
        {
            if (!listener.bPendingRemove && listener.func)
            {
                listener.func(std::forward<Args>(args)...);
            }
        }

        // Broadcast 완료 후 마킹된 listener 정리
        listeners.erase(
            std::remove_if(listeners.begin(), listeners.end(),
                [](const Listener& l) { return l.bPendingRemove; }),
            listeners.end());
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