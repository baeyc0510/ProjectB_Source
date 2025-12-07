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

class CTimeManager : public SingleTon<CTimeManager>
{
	friend SingleTon<CTimeManager>;
private:
	CTimeManager();
	virtual ~CTimeManager();

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

#define DT			CTimeManager::GetInstance()->GetDT()
#define FPS			CTimeManager::GetInstance()->GetFPS()
#define TIMER		CTimeManager::GetInstance()
