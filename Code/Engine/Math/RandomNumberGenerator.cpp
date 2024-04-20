#include "RandomNumberGenerator.hpp"
#include "ThirdParty/Squirrel/Noise/RawNoise.hpp"
int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	//return rand() % maxNotInclusive;

	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_speed);
	return randomUInt % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	int possibilitise = 1 + maxInclusive - minInclusive;
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_speed);

	return minInclusive + (randomUInt % possibilitise);
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	//correct but slow
// 	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_speed);
// 	constexpr unsigned int MAX_RANDOM_UINT = 0xffffffff;
// 	return float(double(randomUInt) / double(MAX_RANDOM_UINT));
	//fast and correct
// 	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_speed);
// 	constexpr unsigned int MAX_RANDOM_UINT = 0xffffffff;
// 	constexpr double ONE_OVER_MAX_RANDOM_UINT = 1.0 / double(MAX_RANDOM_UINT);
// 	return float(double(randomUInt) * ONE_OVER_MAX_RANDOM_UINT);

	return Get1dNoiseZeroToOne(m_position++, m_speed);

}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float randomFloat = Get1dNoiseZeroToOne(m_position++, m_speed);
	float possibilitise = maxInclusive - minInclusive;
	return minInclusive + (randomFloat)*possibilitise;
}

float RandomNumberGenerator::RollRandomFloatNegOneToOne()
{
	return Get1dNoiseNegOneToOne(m_position++, m_speed);
}
