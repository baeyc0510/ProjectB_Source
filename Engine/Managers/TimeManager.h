#pragma once
#include <functional>
#include <unordered_map>

// Timer 전용 핸들 (타입 안전성 확보)
struct TimerHandle
{
	TimerHandle() : handle(0) {}
	TimerHandle(unsigned long long id) : handle(id) {}

	bool IsValid() const { return handle > 0; }
	void Invalidate() { handle = 0; }

	bool operator==(const TimerHandle& other) const
	{
		return handle == other.handle;
	}

	unsigned long long GetHandle() const { return handle; }

private:
	unsigned long long handle;
};

// RAII 패턴 SafeTimerHandle - 소멸 시 자동으로 타이머 정리
// 멤버 변수로 선언하면 객체 파괴 시 자동으로 타이머가 클리어됨
class SafeTimerHandle
{
public:
	SafeTimerHandle() = default;
	~SafeTimerHandle();

	// 복사 금지 (타이머 소유권 명확화)
	SafeTimerHandle(const SafeTimerHandle&) = delete;
	SafeTimerHandle& operator=(const SafeTimerHandle&) = delete;

	// 이동 허용
	SafeTimerHandle(SafeTimerHandle&& other) noexcept;
	SafeTimerHandle& operator=(SafeTimerHandle&& other) noexcept;

	// TimerHandle 할당
	SafeTimerHandle& operator=(const TimerHandle& handle);

	// 명시적 클리어
	void Clear();

	// 유효성 검사
	bool IsValid() const { return handle.IsValid(); }

	// 내부 핸들 접근 (읽기 전용)
	const TimerHandle& Get() const { return handle; }

private:
	TimerHandle handle;
};

// TimerHandle 해시 함수
namespace std
{
	template<>
	struct hash<TimerHandle>
	{
		size_t operator()(const TimerHandle& h) const
		{
			return hash<unsigned long long>()(h.GetHandle());
		}
	};
}

// 타이머 내부 데이터
struct TimerData
{
	std::function<void()>	callback;
	float					rate;
	float					remaining;
	bool					bLoop;
	bool					bPaused;
};

class TimeManager : public SingleTon<TimeManager>
{
	friend SingleTon<TimeManager>;
private:
	TimeManager();
	virtual ~TimeManager();

public:
	void			Init();
	void			Update();
	void			Release();

	// Time
	UINT			GetFPS();
	float			GetDT();
	float			GetUnscaledDT();

	// TimeScale
	void			SetTimeScale(float scale, float duration = 0);
	float			GetTimeScale() const			{ return timeScale; }

	// Timer
	TimerHandle		SetTimer(std::function<void()> callback, float delay, float rate = 0.0f, bool bLoop = false);
	void			ClearTimer(TimerHandle& handle);
	void			ClearAllTimers();
	void			PauseTimer(const TimerHandle& handle);
	void			ResumeTimer(const TimerHandle& handle);
	bool			IsTimerActive(const TimerHandle& handle) const;
	bool			IsTimerPaused(const TimerHandle& handle) const;
	float			GetTimerRemaining(const TimerHandle& handle) const;

private:
	// Time
	UINT			fps;
	float			dt;
	int				updateCount;
	float			updateOneSecond;
	chrono::high_resolution_clock::time_point prevTime;
	chrono::high_resolution_clock::time_point curTime;

	// Timer
	std::unordered_map<TimerHandle, TimerData>	timers;
	std::vector<TimerHandle>					pendingRemove;
	unsigned long long							timerHandleCounter;

	// TimeScale
	float			timeScale;
	float			targetTimeScale;
	float			timeScaleRemaining;
};

#define DT			TimeManager::GetInstance()->GetDT()
#define FPS			TimeManager::GetInstance()->GetFPS()
#define TIMER		TimeManager::GetInstance()
