#include "AnalogJoystick.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	return m_correctedPosition.GetLength();
}

float AnalogJoystick::GetOrientationDegrees() const
{
	return m_correctedPosition.GetOrientationDegrees();
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{

	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2(0.f, 0.f);//SD ToDo:determine reset number
	m_correctedPosition = Vec2(0.f, 0.f);
	m_innerDeadZoneFraction = 0.00f;
	m_outerDeadZoneFraction = 1.f;
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	m_innerDeadZoneFraction = normalizedInnerDeadzoneThreshold;
	m_outerDeadZoneFraction = normalizedOuterDeadzoneThreshold;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	Vec2 rawNormalized = Vec2(rawNormalizedX, rawNormalizedY);
	float normalizedLength = rawNormalized.GetLength();
	if (normalizedLength < m_innerDeadZoneFraction) 
	{
		normalizedLength = 0.f;
	}else if (normalizedLength > m_outerDeadZoneFraction)
	{
		normalizedLength = 1.f;
	}
	else 
	{
		normalizedLength = RangeMapClamped(normalizedLength, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f);
	}
	float theta = Atan2Degrees(rawNormalized.y, rawNormalized.x);
	//Vec2 correctPosition = rawNormalized;
	//correctPosition.SetLength(normalizedLength);
	m_correctedPosition = normalizedLength * Vec2(CosDegrees(theta), SinDegrees(theta));
}
