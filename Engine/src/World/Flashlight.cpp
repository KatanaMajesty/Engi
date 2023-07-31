#include "World/Flashlight.h"

#include "Core/CommonDefinitions.h"
#include "World/SceneRenderer.h"
#include "World/LightSystem/LightManager.h"
#include "Math/Math.h"

namespace engi
{

	Flashlight::Flashlight(SceneRenderer* sceneRenderer)
		: m_sceneRenderer(sceneRenderer)
	{
		ENGI_ASSERT(sceneRenderer);
	}

	Flashlight::~Flashlight()
	{
		this->reset();
	}

	bool Flashlight::init(const Camera* camera, const math::Vec3& color, float intensity, float radius, Texture2D* lightMask) noexcept
	{
		ENGI_ASSERT(camera);
		m_camera = camera;

		this->reset();

		InstanceData cameraData;
		cameraData.modelToWorld = camera->getView();
		cameraData.worldToModel = camera->getViewInv();
		m_cameraInstanceID = m_sceneRenderer->getInstanceTable()->addInstanceData(cameraData);

		LightManager* lm = m_sceneRenderer->getLightManager();
		m_spotLightID = lm->createSpotLight(color, intensity, radius);

		SpotLight& light = lm->getSpotLight(m_spotLightID);
		light.setCookie(lightMask);
		light.setEntityID(m_sceneRenderer->getInstanceTable(), m_cameraInstanceID);
		
		return true;
	}

	void Flashlight::reset() noexcept
	{
		if (this->hasCameraID())
			m_sceneRenderer->getInstanceTable()->removeInstanceData(m_cameraInstanceID);

		if (this->hasSpotlightID())
			m_sceneRenderer->getLightManager()->removeSpotLight(m_spotLightID);
	}

	void Flashlight::update() noexcept
	{
		if (!this->isValid() || !this->hasCameraID())
			return;

		const math::Mat4x4& v = m_camera->getView();
		const math::Mat4x4& vi = m_camera->getViewInv();
		InstanceData& cameraData = m_sceneRenderer->getInstanceTable()->getInstanceData(m_cameraInstanceID);
		cameraData.modelToWorld = vi;
		cameraData.worldToModel = v;

		if (this->hasSpotlightID())
		{
			math::Vec3 front = math::Vec3(v._13, v._23, v._33);
			SpotLight& light = this->getSpotlight();
			light.getDirection() = front;
		}
	}

	SpotLight& Flashlight::getSpotlight() noexcept
	{
		ENGI_ASSERT(this->hasSpotlightID());
		return m_sceneRenderer->getLightManager()->getSpotLight(m_spotLightID);
	}

}; // engi namespace