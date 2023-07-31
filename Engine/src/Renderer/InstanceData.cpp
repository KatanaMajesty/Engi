#include "Renderer/InstanceData.h"

namespace engi
{
	InstanceData::InstanceData(const math::Transformation& transform, const math::Vec3& color, const math::Vec3& emission, float emissionPow)
		: modelToWorld(math::Mat4x4::toWorld(transform.translation, transform.rotation, transform.scale))
		, worldToModel(this->modelToWorld.inverse())
		, color(color)
		, time(0.0f)
		, emission(emission)
		, emissionPower(emissionPow)
		, instanceID(uint32_t(-1))
	{
	}

	std::array<gfx::GpuInputAttributeDesc, 16> InstanceData::getInputAttributes(uint32_t inputSlot) noexcept
	{
		using namespace gfx;
		std::array<GpuInputAttributeDesc, 16> attributes;
		attributes[0] = gfx::GpuInputAttributeDesc("MODEL_TO_WORLD", 0, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, modelToWorld) + 0);
		attributes[1] = gfx::GpuInputAttributeDesc("MODEL_TO_WORLD", 1, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, modelToWorld) + 16);
		attributes[2] = gfx::GpuInputAttributeDesc("MODEL_TO_WORLD", 2, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, modelToWorld) + 32);
		attributes[3] = gfx::GpuInputAttributeDesc("MODEL_TO_WORLD", 3, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, modelToWorld) + 48);
		attributes[4] = gfx::GpuInputAttributeDesc("WORLD_TO_MODEL", 0, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, worldToModel) + 0);
		attributes[5] = gfx::GpuInputAttributeDesc("WORLD_TO_MODEL", 1, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, worldToModel) + 16);
		attributes[6] = gfx::GpuInputAttributeDesc("WORLD_TO_MODEL", 2, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, worldToModel) + 32);
		attributes[7] = gfx::GpuInputAttributeDesc("WORLD_TO_MODEL", 3, GpuFormat::RGBA32F, inputSlot, false, offsetof(InstanceData, worldToModel) + 48);
		attributes[8] = gfx::GpuInputAttributeDesc("INSTANCE_COLOR", 0, GpuFormat::RGB32F, inputSlot, false, offsetof(InstanceData, color));
		attributes[9] = gfx::GpuInputAttributeDesc("INSTANCE_TIME", 0, GpuFormat::R32F, inputSlot, false, offsetof(InstanceData, time));
		attributes[10] = gfx::GpuInputAttributeDesc("INSTANCE_TIMESTEP", 0, GpuFormat::R32F, inputSlot, false, offsetof(InstanceData, timestep));
		attributes[11] = gfx::GpuInputAttributeDesc("INSTANCE_EMISSION", 0, GpuFormat::RGB32F, inputSlot, false, offsetof(InstanceData, emission));
		attributes[12] = gfx::GpuInputAttributeDesc("INSTANCE_EMISSION_POW", 0, GpuFormat::R32F, inputSlot, false, offsetof(InstanceData, emissionPower));
		attributes[13] = gfx::GpuInputAttributeDesc("INSTANCE_SPHERE_ORIGIN", 0, GpuFormat::RGB32F, inputSlot, false, offsetof(InstanceData, sphereOrigin));
		attributes[14] = gfx::GpuInputAttributeDesc("INSTANCE_SPHERE_RADIUS_MAX", 0, GpuFormat::R32F, inputSlot, false, offsetof(InstanceData, sphereRadiusMax));
		attributes[15] = gfx::GpuInputAttributeDesc("INSTANCE_ID", 0, GpuFormat::R32U, inputSlot, false, offsetof(InstanceData, instanceID));
		return attributes;
	}

}; // engi namespace