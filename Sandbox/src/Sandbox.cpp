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

	auto Sphere_Gold = resourcePanel.LoadFromFBX("Sphere_Gold/Sphere_Gold.fbx");
	auto Sphere_Metal = resourcePanel.LoadFromFBX("Sphere_Metal/Sphere_Metal.fbx");
	auto Sphere_Rust = resourcePanel.LoadFromFBX("Sphere_Rust/Sphere_Rust.fbx");
	auto Sphere_Scratched = resourcePanel.LoadFromFBX("Sphere_Scratched/Sphere_Scratched.fbx");
	SceneInspector& sceneInspector = editor->getSceneInspector();

	sceneInspector.AddCamera();
	sceneInspector.SetSkybox("Interstellar_Skybox.dds");
	sceneInspector.SetSceneLight(math::Vec3(1.0f, 2.8f, 4.0f), math::Vec3(0.0f, -0.8f, 0.6f), 1.0f);

	sceneInspector.AddInstance("Sphere_Gold", Sphere_Gold->model, Sphere_Gold->materials, InstanceData(math::Transformation(math::Vec3(0.0f, 0.0f, 0.0f))));
	sceneInspector.AddInstance("Sphere_Metal", Sphere_Metal->model, Sphere_Metal->materials, InstanceData(math::Transformation(math::Vec3(3.0f, 0.0f, 0.0f))));
	sceneInspector.AddInstance("Sphere_Rust", Sphere_Rust->model, Sphere_Rust->materials, InstanceData(math::Transformation(math::Vec3(6.0f, 0.0f, 0.0f))));
	sceneInspector.AddInstance("Sphere_Scratched", Sphere_Scratched->model, Sphere_Scratched->materials, InstanceData(math::Transformation(math::Vec3(9.0f, 0.0f, 0.0f))));

	// uint32_t numKnights = 2;
	// for (uint32_t x = 0; x < numKnights; ++x)
	// {
	// 	for (uint32_t z = 0; z < numKnights; ++z)
	// 	{
	// 		std::string instanceName = "Knight" + std::to_string(x + z);
	// 		sceneInspector.AddInstance(instanceName, knight->model, knight->materials, InstanceData(math::Transformation(math::Vec3(x * 2.0f, 0.0f, z * 2.0f))));
	// 	}
	// }
	// 
	// uint32_t numSamurais = 2;
	// for (uint32_t x = 0; x < numSamurais; ++x)
	// {
	// 	for (uint32_t z = 0; z < numSamurais; ++z)
	// 	{
	// 		std::string instanceName = "Samurai" + std::to_string(x + z);
	// 		sceneInspector.AddInstance(instanceName, samurai->model, samurai->materials, InstanceData(math::Transformation(math::Vec3(x * 2.0f, 2.0f, z * 2.0f))));
	// 	}
	// }

	MaterialInstance floorMaterial("Floor_Material", resourcePanel.GetMaterial(MATERIAL_BRDF_PBR));
	floorMaterial.setTexture(TEXTURE_ALBEDO, resourcePanel.GetTexture2D("Rock_CliffVolcanic_albedo.dds"));
	floorMaterial.setTexture(TEXTURE_NORMAL, resourcePanel.GetTexture2D("Rock_CliffVolcanic_normal.dds"));
	floorMaterial.setTexture(TEXTURE_ROUGHNESS, resourcePanel.GetTexture2D("Rock_CliffVolcanic_roughness.dds"));
	floorMaterial.setMetallic(0.1f);
	sceneInspector.AddCube("Floor", floorMaterial, math::Transformation(
		math::Vec3(0.0f, -0.5f, 0.0f),
		math::Vec3(0.0f),
		math::Vec3(40.0f, 0.05f, 20.0f)
	));

	sceneInspector.AddPointLight("Point Light 1", math::Vec3(-1.5f, 0.5f, -1.0f), math::Vec3(0.9f, 0.1f, 0.6f), 32.0f, 0.3f);
	sceneInspector.AddPointLight("Point Light 2", math::Vec3(1.5f, 1.5f, 1.0f), math::Vec3(0.2f, 0.2f, 0.6f), 64.0f, 0.3f);

	EmitterSettings& globalEmitterSettings = ParticleSystem::getGlobalEmitterSettings();
	globalEmitterSettings.numParticles = 200;
	globalEmitterSettings.color = math::Vec3(1.0f);
	globalEmitterSettings.minimumSpeed = math::Vec3(-0.2f, 1.5f, -0.2f);
	globalEmitterSettings.maximumSpeed = math::Vec3(0.2f, 2.5f, 0.2f);
	globalEmitterSettings.minimumParticleSize = 0.2f;
	globalEmitterSettings.maximumParticleSize = 0.3f;
	globalEmitterSettings.minimumParticleGrowth = 0.1f;
	globalEmitterSettings.maximumParticleGrowth = 0.2f;
	globalEmitterSettings.minimumLifetime = 3.0f;
	globalEmitterSettings.maximumLifetime = 4.5f;
	globalEmitterSettings.spawnRate = 0.1f;

	Texture2D* mvea = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_MVEA.dds", 8, 8);
	Texture2D* dbf = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_DBF.dds", 8, 8);
	Texture2D* rlu = resourcePanel.GetTextureAtlas("Particles/Smoke/smoke_RLU.dds", 8, 8);
	sceneInspector.AddSmokeEmitter("Emitter 1", mvea, dbf, rlu, math::Transformation(math::Vec3(), math::Vec3(), math::Vec3(0.3f)), math::Vec3(0.5f, 2.4f, 1.2f));

	// sceneInspector.AddFlashLight(math::Vec3(0.5f, 1.0f, 4.0f), 8.0f, 1.0f, resourcePanel.GetTexture2D("Spotlight_Mask.dds"));
}
