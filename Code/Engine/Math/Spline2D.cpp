#include "Engine/Math/Spline2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicHermiteCurve2D.hpp"
Spline2D::Spline2D(std::vector<Vec2> const& positions) : m_posiArray(positions)
{
	for (int veloIndex = 0; veloIndex < (int)m_posiArray.size(); veloIndex++) 
	{
		if (veloIndex == 0 || veloIndex == (int)m_posiArray.size() - 1)
		{
			m_veloArray.push_back(Vec2(0.f, 0.f));
		}
		else//velocity[i] = (position[i+1] ¨C position[i-1]) / 2
		{
			Vec2 thisVelo = (m_posiArray[veloIndex + 1] - m_posiArray[veloIndex - 1]) * 0.5f;
			m_veloArray.push_back(thisVelo);
		}
	}

	for (int posiIndex = 0; posiIndex < (int)m_posiArray.size(); posiIndex++) 
	{
		if (posiIndex + 1 < (int)m_posiArray.size()) 
		{
			CubicHermiteCurve2D* hermiteCurve = new CubicHermiteCurve2D(m_posiArray[posiIndex], m_veloArray[posiIndex], m_veloArray[posiIndex + 1], m_posiArray[posiIndex + 1]);
			m_hermiteArray.push_back(hermiteCurve);
		}
	}

}

Spline2D::~Spline2D()
{

}



Vec2 Spline2D::EvaluateAtParametricForEachHermite(float t) const
{
	float unitT = fmodf(t, (float)m_hermiteArray.size());//10.14 mod 4 = 2.14
	float fractionT = unitT - RoundDownToInt(unitT);	 //2.14 -2=0.14
	Vec2 result = m_hermiteArray[RoundDownToInt(unitT)]->EvaluateAtParametric(fractionT);
	return result;
}



Vec2 Spline2D::EvaluateAtApproximateDistForEachHermite(float t, int numSubdivisions /*= 64*/) const
{
	float unitT = fmodf(t, (float)m_hermiteArray.size());//10.14 mod 4 = 2.14
	float speed = m_hermiteArray[RoundDownToInt(unitT)]->GetApproximateLength(numSubdivisions);// /1.f=speed
	float fractionT = unitT - RoundDownToInt(unitT);
	float distanceAlongCurve = fractionT * speed;

	Vec2 result = m_hermiteArray[RoundDownToInt(unitT)]->EvaluateAtApproximateDistance(distanceAlongCurve, numSubdivisions);
	return result;
}
