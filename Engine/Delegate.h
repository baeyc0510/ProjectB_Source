#pragma once
#include <vector>
#include <functional>
#include <algorithm>

// 핸들 타입
using DelegateHandle = unsigned long long;

template<typename... Args>
class Delegate
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

    std::vector<Listener> m_listeners;
    DelegateHandle       m_idCounter = 0; // 핸들 발급용 generator

public:
    Delegate() {}
    ~Delegate() { Clear(); }

    DelegateHandle Add(EventFunc func)
    {
        DelegateHandle newHandle = ++m_idCounter;
        m_listeners.push_back({ newHandle, func });
        return newHandle;
    }

    void Remove(DelegateHandle& handle)
    {
        if (handle == 0) return;

        auto iter = std::remove_if(m_listeners.begin(), m_listeners.end(),
            [handle](const Listener& listener) 
            {
                return listener.handle == handle;
            });

        // 실제로 삭제된 항목이 있을 때만 erase
        if (iter != m_listeners.end())
        {
            m_listeners.erase(iter, m_listeners.end());
            handle = 0; // 핸들 무효화
        }
    }

    void Clear()
    {
        m_listeners.clear();
    }

    void Broadcast(Args... args)
    {
        if (m_listeners.empty()) 
            return;

        for (int i = m_listeners.size() - 1; i >= 0; --i)
        {
            const auto& listener : m_vecListeners[i];
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
        return !m_listeners.empty(); 
    }
};