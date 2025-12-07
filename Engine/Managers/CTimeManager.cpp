#include "pch.h"
#include "CTimeManager.h"

CTimeManager::CTimeManager()
{
	fps = 1;
	dt = 1;
	updateCount = 0;
	updateOneSecond = 0;

	prevTime = {};
	curTime = {};

	timerHandleCounter = 0;

	timeScale = 1.0f;
	targetTimeScale = 1.0f;
	timeScaleRemaining = 0;
}

CTimeManager::~CTimeManager()
{
}

void CTimeManager::Init()
{
	prevTime = chrono::high_resolution_clock::now();
	curTime = chrono::high_resolution_clock::now();

	timerHandleCounter = 0;
	timers.clear();
	pendingRemove.clear();
}

void CTimeManager::Update()
{
	// 이전프레임에서 현재프레임까지 걸린 시간을 측정
	// 단위시간 = 현재프레임시간 - 이전프레임시간
	curTime = chrono::high_resolution_clock::now();
	chrono::duration<float> elapsed = curTime - prevTime;

	dt = elapsed.count();
	if (dt > 0.1f) dt = 0.1f;
	prevTime = curTime;

	// 1초가 걸릴때까지 반복한 횟수가 초당프레임수
	updateCount++;
	updateOneSecond += dt;
	if (updateOneSecond >= 1.0)
	{
		fps = updateCount;
		updateOneSecond = 0;
		updateCount = 0;
	}

	// TimeScale duration 처리 (unscaled dt 사용)
	if (timeScaleRemaining > 0)
	{
		timeScaleRemaining -= dt;
		if (timeScaleRemaining <= 0)
		{
			timeScale = targetTimeScale;
			timeScaleRemaining = 0;
		}
	}

	// Timer 업데이트
	for (auto& handle : pendingRemove)
	{
		timers.erase(handle);
	}
	pendingRemove.clear();

	for (auto& [handle, timer] : timers)
	{
		if (timer.bPaused)
			continue;

		timer.remaining -= dt;

		if (timer.remaining <= 0.0f)
		{
			if (timer.callback)
			{
				timer.callback();
			}

			if (timer.bLoop)
			{
				timer.remaining = timer.rate;
			}
			else
			{
				pendingRemove.push_back(handle);
			}
		}
	}
}

void CTimeManager::Release()
{
	ClearAllTimers();
}

UINT CTimeManager::GetFPS()
{
	return fps;
}

float CTimeManager::GetDT()
{
	return dt * timeScale;
}

float CTimeManager::GetUnscaledDT()
{
	return dt;
}

void CTimeManager::SetTimeScale(float scale, float duration)
{
	if (duration <= 0)
	{
		timeScale = scale;
		targetTimeScale = scale;
		timeScaleRemaining = 0;
	}
	else
	{
		timeScale = scale;
		targetTimeScale = 1.0f;
		timeScaleRemaining = duration;
	}
}

TimerHandle CTimeManager::SetTimer(std::function<void()> callback, float delay, float rate, bool bLoop)
{
	TimerHandle newHandle(++timerHandleCounter);

	TimerData data;
	data.callback = callback;
	data.rate = rate;
	data.remaining = delay;
	data.bLoop = bLoop;
	data.bPaused = false;

	timers[newHandle] = data;

	return newHandle;
}

void CTimeManager::ClearTimer(TimerHandle& handle)
{
	if (!handle.IsValid())
		return;

	auto iter = timers.find(handle);
	if (iter != timers.end())
	{
		timers.erase(iter);
	}

	handle.Invalidate();
}

void CTimeManager::ClearAllTimers()
{
	timers.clear();
	pendingRemove.clear();
	timerHandleCounter = 0;
}

void CTimeManager::PauseTimer(const TimerHandle& handle)
{
	auto iter = timers.find(handle);
	if (iter != timers.end())
	{
		iter->second.bPaused = true;
	}
}

void CTimeManager::ResumeTimer(const TimerHandle& handle)
{
	auto iter = timers.find(handle);
	if (iter != timers.end())
	{
		iter->second.bPaused = false;
	}
}

bool CTimeManager::IsTimerActive(const TimerHandle& handle) const
{
	return timers.find(handle) != timers.end();
}

bool CTimeManager::IsTimerPaused(const TimerHandle& handle) const
{
	auto iter = timers.find(handle);
	if (iter != timers.end())
	{
		return iter->second.bPaused;
	}
	return false;
}

float CTimeManager::GetTimerRemaining(const TimerHandle& handle) const
{
	auto iter = timers.find(handle);
	if (iter != timers.end())
	{
		return iter->second.remaining;
	}
	return 0.0f;
}
