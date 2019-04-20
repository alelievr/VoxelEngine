#include "VoxelEngine.hpp"
#include "VoxelRenderPipeline.hpp"

VoxelEngine::VoxelEngine(TerrainSettings & settings)
{
	Application		app;
	EventSystem *	es = app.GetEventSystem();
	Hierarchy *		hierarchy = app.GetHierarchy();

	ShaderSource::AddIncludePath("./");
	ShaderSource::AddIncludePath("Deps/LWGC/");
	ShaderSource::AddIncludePath("Deps/LWGC/Shaders/");
	ShaderSource::AddIncludePath("Shaders/");

	Load(settings);

	RenderPipelineManager::SetCurrentRenderPipeline(new VoxelRenderPipeline());

	app.Init();
	RegisterEvents(es, &app);

	// We must Open the window before doing anything related to vulkan
	app.Open("Vox", 1920, 1200, WindowFlag::Resizable | WindowFlag::Decorated | WindowFlag::Focused);

	auto cam = new GameObject(new Camera());
	cam->GetTransform()->SetPosition(glm::vec3(0, 0, -5));
	cam->AddComponent(new FreeCameraControls());
	_gui = new GameObject(new ProfilerPanel());

	hierarchy->AddGameObject(_gui);
	hierarchy->AddGameObject(cam);

	while (app.ShouldNotQuit())
	{
		app.Update();
	}
}

void	VoxelEngine::Load(TerrainSettings & settings)
{
	(void)settings;
	std::cout << "Load setting file: TODO" << std::endl;
}

void	VoxelEngine::RegisterEvents(EventSystem * es, Application * app)
{
	es->Get()->onKey.AddListener([&](KeyCode key, ButtonAction action, int)
		{
			if (action == ButtonAction::Press && key == KeyCode::ESCAPE)
				app->Quit();
		}
	);
}
