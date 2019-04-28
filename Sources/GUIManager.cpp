#include "GUIManager.hpp"

TerrainSettings *	GUIManager::currentSettings;
GameObject *		GUIManager::parent;

using namespace LWGC;

void		GUIManager::Initialize(void) noexcept
{
	Application * app = Application::Get();

	parent = new GameObject();

	// Add all required GUI panels:
	parent->AddComponent(new ImGUIPanel(NoiseSettingsPanel));

	app->GetHierarchy()->AddGameObject(parent);
}

void		GUIManager::NoiseSettingsPanel()
{
	bool opened = true;

	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &opened, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::Begin("Voxel Terrain Settings", &opened, ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::TextColored(ImVec4{1, 0, 1, 1}, "");

	ImGui::End();

	ImGui::End();
}

void		GUIManager::LoadSettings(TerrainSettings * settings) noexcept
{
	currentSettings = settings;
}
