#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
StopWatch::StopWatch(float duration)
	:m_duration(duration)
{
	m_clock = (const Clock*)&Clock::GetSystemClock();
	if (m_duration == 0) {
		m_invDuration = 1.f;
	}
	else
	{
		m_invDuration = 1.f / m_duration;
	}
}

StopWatch::StopWatch(const Clock* clock, float duration)
{
	m_clock = clock;
	m_duration = duration;
	if (m_duration == 0) {
		m_invDuration = 1.f;
	}
	else
	{
		m_invDuration = 1.f / m_duration;
	}
}

void StopWatch::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
	m_invDuration = 1.f / m_duration;
}

void StopWatch::Restart()
{
	if (!IsStopped())
	{
		Start();
	}
	// if (m_startTime + m_duration <= m_clock->GetTotalSeconds()) 
	// {
	// 	m_startTime = m_clock->GetTotalSeconds();
	// }
}

void StopWatch::Stop()
{
	m_startTime = -1.f;
}

float StopWatch::GetElapsedTime() const
{
// 	if (m_startTime + m_duration < m_clock->GetTotalSeconds())  // haven't reach end
// 	{
// 		return (m_clock->GetTotalSeconds() - m_startTime);
// 	}
// 	else
// 	{
// 		return 0.f;
// 	}
	if (IsStopped()) return 0.f;

	return (m_clock->GetTotalSeconds() - m_startTime);
}

float StopWatch::GetElapsedFraction() const
{
// 	if (m_startTime + m_duration < m_clock->GetTotalSeconds())  // haven't reach end
// 	{
// 		return ((m_clock->GetTotalSeconds() - m_startTime) / m_duration);
// 	}
// 	else
// 	{
// 		return 0.f;
// 	}
	return GetElapsedTime() * m_invDuration;
}

bool StopWatch::IsStopped() const
{
	return m_startTime == 0.f;
}

bool StopWatch::HasDurationElapsed() const
{
	return (GetElapsedTime() > m_duration);
}

bool StopWatch::DecrementDurationIfElapsed()
{
	if (HasDurationElapsed()) 
	{
		m_startTime += m_duration;
		return true;
	}
	return false;
}
