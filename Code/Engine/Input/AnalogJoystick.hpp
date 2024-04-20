#pragma once
#include"Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"

class AnalogJoystick
{
	friend class XboxController;
public:
	Vec2 GetPosition() const; 
	float GetMagnitude() const;
	float GetOrientationDegrees() const;
	Vec2 GetRawUncorrectedPosition() const; 
	float GetInnerDeadZoneFraction() const;
	float GetOuterDeadZoneFraction() const;
	//For use by XboxController,et al.
	void Reset();
	void SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold); 
	void UpdatePosition(float rawNormalizedX, float rawNormalizedY);
protected:
	Vec2 m_rawPosition;											// Flaky; doesn't rest at zero (or consistently snap to rest position)	[-1,1]
	Vec2 m_correctedPosition;									//Deadzone-corrected position	[-1,1]
	float m_innerDeadZoneFraction = 0.00f;						// if R < this%, R = 0;"input range start" for corrective range map	[0,1]
	float m_outerDeadZoneFraction = 1.00f;						// if R> this%, R = 1; "input range end"for corrective range map	[0,1]
};
