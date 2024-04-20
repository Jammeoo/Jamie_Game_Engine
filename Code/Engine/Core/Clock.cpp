#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <algorithm>

static Clock s_systemClock;

Clock::Clock()
{
	if (this != &s_systemClock)
	{
		m_parent = &s_systemClock;
		s_systemClock.AddChild(this);
	}
}

Clock::Clock(Clock& parent)
	:m_parent(&parent)
{
	parent.AddChild(this);
}

Clock::~Clock()
{
	for (int childIndex = 0; childIndex < (int)m_children.size(); childIndex++) 
	{
		m_children[childIndex] = nullptr;
	}
}

void Clock::Reset()
{
	m_totalSeconds = 0.0f;
	m_deltaSeconds = 0.0f;
	m_frameCount = 0;
	m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Paused()
{
	m_isPaused = true;
}

void Clock::UnPaused()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_stepSingleFrame = !m_stepSingleFrame;
// 	if (m_stepSingleFrame) 
// 	{
// 		m_isPaused = false;
// 	}
	m_isPaused = false;
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	s_systemClock.Tick();
}

void Clock::Tick()
{
	//calculate current delta seconds
	float timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaSeconds = timeNow - m_lastUpdateTimeInSeconds;
	if (deltaSeconds > m_maxDeltaSeconds)
	{
		deltaSeconds = m_maxDeltaSeconds;
	}
	//Update book keeping variables
	Advance(deltaSeconds);
	m_lastUpdateTimeInSeconds = timeNow;
}

void Clock::Advance(float deltaSeconds)
{
	//Update this book keeping variables
	if (m_isPaused == true) 
	{
		m_deltaSeconds = 0.f;
	}
	else
	{
		m_deltaSeconds = deltaSeconds * m_timeScale;
		m_totalSeconds += m_deltaSeconds;
	}
	m_frameCount++;
	
	//Update children's book keeping variables
	for (int childIndex = 0; childIndex < (int)m_children.size(); childIndex++) 
	{
		m_children[childIndex]->Advance(m_deltaSeconds);
	}

	if (m_stepSingleFrame) 
	{
		m_isPaused = true;
		m_stepSingleFrame = false;
	}
	//DebuggerPrintf("m_deltaSeconds = %f m_lastUpdateTimeInSeconds=%f m_totalSeconds=%f\n ", m_deltaSeconds, m_lastUpdateTimeInSeconds, m_totalSeconds);
}

void Clock::AddChild(Clock* childClock)
{
	m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
	m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [=](auto const& element)
		{
			return element == childClock;
		}),
	m_children.end());
}

