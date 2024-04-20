#pragma once
#include <vector>
struct Vec2;
class CubicHermiteCurve2D;

class Spline2D			//fully-welded, continuous Cubic Hermite Spline
{
public:
	Spline2D(std::vector<Vec2> const& positions);
	~Spline2D();
	Vec2 EvaluateAtParametricForEachHermite(float t)const;
	Vec2 EvaluateAtApproximateDistForEachHermite(float t, int numSubdivisions = 64)const;
public:
	std::vector<Vec2> m_posiArray;
	std::vector<Vec2> m_veloArray;

	std::vector<CubicHermiteCurve2D*> m_hermiteArray;
private:
};

