#pragma once
class Clock;

class StopWatch
{
public:
	explicit StopWatch(float duration);
	StopWatch(const Clock* clock, float duration);
	void Start();
	void Restart();
	void Stop();
	float GetElapsedTime()const;
	float GetElapsedFraction()const;
	bool IsStopped()const;
	bool HasDurationElapsed()const;
	bool DecrementDurationIfElapsed();

	const Clock* m_clock = nullptr;
	float m_startTime = 0.0f;
	float m_duration = 0.0f;
	float m_invDuration = 0.f;
};