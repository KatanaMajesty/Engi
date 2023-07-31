#include <Engi.h>

#include <iostream>
#include <format>

#define KNIGHT_INSTANCE_TESTING() (resourcePanel.LoadFromFBX("Knight/Knight.fbx"))
#define SAMURAI_INSTANCE_TESTING() (resourcePanel.LoadFromFBX("Samurai/Samurai.fbx"))

void TestFibonacciPointDistribution(uint32_t n) noexcept
{
#ifndef NDEBUG
	using namespace engi;
	float sum = 0.0f;
	for (uint32_t i = 0; i < n; ++i)
	{
		float NdotL;
		math::fibonacciHemispherePoint(NdotL, i, n);
		sum += NdotL;
	}
	sum *= math::Numeric::pi2() / n;
	ENGI_LOG_INFO("Result of Fibonacci point distribution is {:.7f}", sum);
#endif
}

void engi::Main(char* cmdline, int32_t cmdshow)
{
	// TestFibonacciPointDistribution(300);

	Application& app = Application::get();
	app.init(WindowSpecs("DX11 Engi Window", 800, 600));

	Editor* editor = app.getEditor();
	ResourcePanel& resourcePanel = editor->getResourcePanel();
	auto knight = resourcePanel.LoadFromFBX("Knight/Knight.fbx");
	auto samurai = resourcePanel.LoadFromFBX("Samurai/Samurai.fbx");
	SceneInspector& sceneInspector = editor->getSceneInspector();

	MaterialInstance knightHolo("Knight_Holo_Mat", resourcePanel.GetMaterial(MATERIAL_HOLOGRAM));
	InstanceData holoData(math::Transformation(), math::Vec3(), math::Vec3(1.0f, 0.0f, 0.0f), 2.0f);

	sceneInspector.AddCamera();
	sceneInspector.SetSkybox("Interstellar_Skybox.dds");
	sceneInspector.SetSceneLight(math::Vec3(1.0f, 2.8f, 4.0f), math::Vec3(0.0f, -0.8f, 0.6f), 1.0f);
	sceneInspector.AddInstance("Knight1", knight->model, viewOf(knightHolo), holoData);
	sceneInspector.AddInstance("Knight2", knight->model, knight->materials, InstanceData(math::Transformation(math::Vec3(2.0f, 0.0f, 0.0f))));
	sceneInspector.AddInstance("Knight3", knight->model, knight->materials, InstanceData(math::Transformation(math::Vec3(-2.0f, 0.0f, 0.0f))));
	sceneInspector.AddInstance("Samurai", samurai->model, samurai->materials, InstanceData(math::Transformation(math::Vec3(4.0f, 0.0f, 0.0f))));

	MaterialInstance floorMaterial("Floor_Material", resourcePanel.GetMaterial(MATERIAL_BRDF_PBR));
	floorMaterial.setTexture(TEXTURE_ALBEDO, resourcePanel.GetTexture2D("Rock_CliffVolcanic_albedo.dds"));
	floorMaterial.setTexture(TEXTURE_NORMAL, resourcePanel.GetTexture2D("Rock_CliffVolcanic_normal.dds"));
	floorMaterial.setTexture(TEXTURE_ROUGHNESS, resourcePanel.GetTexture2D("Rock_CliffVolcanic_roughness.dds"));
	floorMaterial.setMetallic(0.1f);
	sceneInspector.AddCube("Floor", floorMaterial, math::Transformation(
		math::Vec3(0.0f, -0.5f, 0.0f),
		math::Vec3(0.0f),
		math::Vec3(20.0f, 0.05f, 20.0f)
	));

	sceneInspector.AddPointLight("Point Light 1", math::Vec3(-1.5f, 0.5f, -1.0f), math::Vec3(0.2f, 2.0f, 4.0f), 64.0f, 0.1f);

	EmitterSettings& globalEmitterSettings = ParticleSystem::getGlobalEmitterSettings();
	globalEmitterSettings.numParticles = 100;
	globalEmitterSettings.color = math::Vec3(1.0f);
	globalEmitterSettings.minimumSpeed = math::Vec3(-0.2f, 1.5f, -0.2f);
	globalEmitterSettings.maximumSpeed = math::Vec3(0.2f, 2.5f, 0.2f);
	globalEmitterSettings.minimumParticleSize = 0.2f;
	globalEmitterSettings.maximumParticleSize = 0.3f;
	globalEmitterSettings.minimumParticleGrowth = 0.1f;
	globalEmitterSettings.maximumParticleGrowth = 0.2f;
	globalEmitterSettings.minimumLifetime = 3.0f;
	globalEmitterSettings.maximumLifetime = 4.5f;
	globalEmitterSettings.spawnRate = 0.8f;

	Texture2D* mvea = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_MVEA.dds", 8, 8);
	Texture2D* dbf = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_DBF.dds", 8, 8);
	Texture2D* rlu = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_RLU.dds", 8, 8);
	sceneInspector.AddSmokeEmitter("Emitter 1", mvea, dbf, rlu, math::Transformation(math::Vec3(), math::Vec3(), math::Vec3(0.1f)), math::Vec3(0.5f, 2.4f, 1.2f));

	// sceneInspector.AddFlashLight(math::Vec3(0.5f, 1.0f, 4.0f), 8.0f, 1.0f, resourcePanel.GetTexture2D("Spotlight_Mask.dds"));
}
