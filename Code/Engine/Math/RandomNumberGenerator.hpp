#pragma once 
#include<cstdlib>
class RandomNumberGenerator {

public:
	RandomNumberGenerator() = default;
	RandomNumberGenerator(unsigned int seed) :m_speed(seed){}

	int   RollRandomIntLessThan(int maxNotInclusive );
	int   RollRandomIntInRange(int minInclusive, int maxInclusive );
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive );
	float RollRandomFloatNegOneToOne();
	void SetSeed(unsigned int newSeed) { m_speed = newSeed; }
public:
	unsigned int m_speed = 0;
	int m_position = 0;
};