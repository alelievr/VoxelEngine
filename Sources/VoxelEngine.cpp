#include "VoxelEngine.hpp"
#include "VoxelRenderPipeline.hpp"
#include "GUIManager.hpp"
#include "AssetManager.hpp"

VoxelEngine::VoxelEngine(TerrainSettings & settings)
{
	Application		app;
	EventSystem *	es = app.GetEventSystem();
	Hierarchy *		hierarchy = app.GetHierarchy();

	ShaderSource::AddIncludePath("./");
	ShaderSource::AddIncludePath("Deps/LWGC/");
	ShaderSource::AddIncludePath("Deps/LWGC/Shaders/");
	ShaderSource::AddIncludePath("Shaders/");

	RenderPipelineManager::SetCurrentRenderPipeline(new VoxelRenderPipeline());

	app.Init();
	RegisterEvents(es, &app);

	// We must Open the window before doing anything related to vulkan
	app.Open("Vox", 1920, 1200, WindowFlag::Resizable | WindowFlag::Decorated | WindowFlag::Focused);

	GUIManager::Initialize();

	Load(settings);

	AssetManager::LoadAssets();

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
	exit(0); // TODO: remove this
}

void	VoxelEngine::Load(TerrainSettings & settings)
{
	(void)settings;
	std::cout << "Load setting: TODO" << std::endl;

	GUIManager::LoadSettings(&settings);
}

void	VoxelEngine::RegisterEvents(EventSystem * es, Application * app)
{
	es->Get()->onKey.AddListener([&](KeyCode key, ButtonAction action, int mods)
		{
			if (action == ButtonAction::Press && key == KeyCode::ESCAPE)
			{
				if (mods == 2) // Control, TODO: LWGC defines for key mods
					exit(0);
				else
					app->Quit();
			}
		}
	);
}
