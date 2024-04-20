#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>

class HeatMaps
{
public:
	HeatMaps(IntVec2 const& dimensions, float initialValue);
	~HeatMaps();
	void SetAllValues(float valuesToSet);
	float GetHeatValuesAt(IntVec2 const& tileCoords);
	void SetHeatValuesAt(IntVec2 const& tileCoords, float heatValueToSet);
	void AddHeatValuesAt(IntVec2 const&, float heatValueToAdd);
	

public:
	std::vector<float> m_values;
	float m_initialValue = 0.f;
	IntVec2 m_dimensions = IntVec2::ZERO;
};
