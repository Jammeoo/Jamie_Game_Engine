#pragma once
#include <vector>
// [2/7/2023 Jianming Shi]
//  Hierarchical clock that inherits time scale.Parent clocks pass scaled delta seconds down to 
//  child clocks to be used as their base delta seconds.Child clocks in turn scale that time and
//  pass that down to their children.There is one system clock at the root of the hierarchy.
class Clock
{
public:
	Clock();
	explicit Clock(Clock& parent);

	~Clock();
	Clock(const Clock& copy) = delete;
	void Reset();
	bool IsPaused() const;
	void Paused();
	void UnPaused();
	void TogglePause();

	void StepSingleFrame();
	void SetTimeScale(float timeScale);
	float GetTimeScale()const;

	float GetDeltaSeconds()const;
	float GetTotalSeconds()const;
	size_t GetFrameCount()const;

public:
	static Clock& GetSystemClock();

	static void TickSystemClock();

protected:
	void Tick();

	void Advance(float deltaSeconds);
	void AddChild(Clock* childClock);
	void RemoveChild(Clock* childClock);

protected:
	Clock* m_parent = nullptr;
	std::vector<Clock*> m_children;

	float	m_lastUpdateTimeInSeconds = 0.0f;
	float	m_totalSeconds = 0.0f;				
	float	m_deltaSeconds = 0.0f;
	size_t  m_frameCount = 0;

	float	m_timeScale = 1.f;
	bool	m_isPaused = false;
	bool	m_stepSingleFrame = false;
	float	m_maxDeltaSeconds = 0.1f;

};