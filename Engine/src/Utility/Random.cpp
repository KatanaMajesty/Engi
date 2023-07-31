#include "Random.h"

#include <random>
#include "Core/CommonDefinitions.h"

namespace engi
{
	std::random_device randomDevice;
	std::mt19937 generator = std::mt19937(randomDevice());

	std::uniform_real_distribution<float> distributionFloat(0.0f, 1.0f);
	

	float Random::GenerateFloat() noexcept
	{
		return distributionFloat(generator);
	}

	float Random::GenerateFloat(float L, float R) noexcept
	{
		float d = R - L;
		return L + Random::GenerateFloat() * d;
	}

	math::Vec3 Random::GenerateFloat3(const math::Vec3& L, const math::Vec3& R) noexcept
	{
		return math::Vec3(GenerateFloat(L.x, R.x), GenerateFloat(L.y, R.y), GenerateFloat(L.z, R.z));
	}

	uint32_t Random::GenerateUnsigned(uint32_t L, uint32_t R) noexcept
	{
		ENGI_ASSERT(R > L);
		std::uniform_int<uint32_t> distributionUnsignedInt(L, R);
		return distributionUnsignedInt(generator);
	}

}; // engi namespace