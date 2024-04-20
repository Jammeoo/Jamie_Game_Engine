#include "Engine/Core/HeatMaps.hpp"

HeatMaps::HeatMaps(IntVec2 const& dimensions,float initialValue)
	:m_dimensions(dimensions)
{
	int tileNum = dimensions.x * dimensions.y;
	m_values.resize(tileNum);
	SetAllValues(initialValue);
}

HeatMaps::~HeatMaps()
{

}

void HeatMaps::SetAllValues(float valuesToSet)
{
	int maxAmount = (int)m_values.size();
	for (int index = 0; index < maxAmount; index++) 
	{
		m_values[index] = valuesToSet;
	}
}

float HeatMaps::GetHeatValuesAt(IntVec2 const& tileCoords)
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	return m_values[tileIndex];
}

void HeatMaps::SetHeatValuesAt(IntVec2 const& tileCoords, float heatValueToSet)
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	m_values[tileIndex] = heatValueToSet;
}

void HeatMaps::AddHeatValuesAt(IntVec2 const& tileCoords, float heatValueToAdd)
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	m_values[tileIndex] += heatValueToAdd;
}

