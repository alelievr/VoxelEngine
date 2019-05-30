#include "GUIManager.hpp"

TerrainSettings *	GUIManager::currentSettings;
GameObject *		GUIManager::parent;

using namespace LWGC;

const char *noiseTypeEnumNames[] = {"Perlin", "Simplex", "Voronoi"};

void		GUIManager::Initialize(void) noexcept
{
	Application * app = Application::Get();

	parent = new GameObject();

	// Add all required GUI panels:
	parent->AddComponent(new ImGUIPanel(DockManager));

	app->GetHierarchy()->AddGameObject(parent);
}

void		GUIManager::DockManager(void) noexcept
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
	ImGui::Begin("DockSpace", &opened, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	DrawTerrainSettingsGUI();

	ImGui::End();
}

#define NAME_BUFFER_SIZE	127
static char nameBuffer[NAME_BUFFER_SIZE + 1];
void		GUIManager::DrawTerrainSettingsGUI(void) noexcept
{
	bool opened = true; // TODO


	ImGui::Begin("Voxel Terrain Settings", &opened, ImGuiDockNodeFlags_PassthruCentralNode);
	{
		ImGui::InputInt("Global Seed", &currentSettings->globalSeed, 1, 10);
		ImGui::InputInt("Chunk Size", &currentSettings->chunkSize, 1, 10);

		if (ImGui::Button("Add Noise"))
			currentSettings->noiseSettings.push_back(NoiseSettings{});

		if (ImGui::Button("Save")) {
			// currentSettings->Save()
			ImGui::TextColored(ImVec4{1, 0, 1, 1}, "OK");
		}

		ImGui::Text("Noise Settings List");
		if (ImGui::ListBoxHeader("##empty", ImVec2(-1, -1)))
		{
			for (size_t id = 0; id < currentSettings->noiseSettings.size(); id++)
			{
				DrawNoiseSettingItem(id);
			}
			ImGui::ListBoxFooter();
		}
	}
	ImGui::End();
}

void		GUIManager::DrawNoiseSettingItem(size_t id)
{
	auto & noiseSetting = currentSettings->noiseSettings[id];
	size_t noiseType = static_cast< int >(noiseSetting.type);

	ImGui::PushID(id);
	strncpy(nameBuffer, noiseSetting.name.data(), NAME_BUFFER_SIZE);
	
	ImGui::Selectable(nameBuffer, false);
	
	if (ImGui::InputText("Name", nameBuffer, NAME_BUFFER_SIZE))
		noiseSetting.name = std::string(nameBuffer);

	ImGui::InputInt("Octaves", &noiseSetting.octaves, 1, 10);
	ImGui::InputFloat("Turbulance", &noiseSetting.turbulance, 1, 10);
		
	if (ImGui::BeginCombo("Noise", noiseTypeEnumNames[noiseType], 0))
	{
		for (size_t comboId = 0; comboId < 3; comboId++)
		{
			ImGui::PushID(comboId);
			const bool item_selected = (noiseType == comboId);
			const char* item_text;

			if (noiseType >= 3)
				item_text = "*Unknown item*";
			else
				item_text = noiseTypeEnumNames[comboId];

			if (ImGui::Selectable(item_text, item_selected))
				noiseSetting.type = static_cast< NoiseType >(comboId);
			
			if (item_selected)
				ImGui::SetItemDefaultFocus();
			
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}


    // if (!BeginCombo(label, preview_value, ImGuiComboFlags_None))
    //     return false;

    // Display items
    // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
    // bool value_changed = false;
    // for (int i = 0; i < items_count; i++)
    // {
    //     PushID((void*)(intptr_t)i);
    //     const bool item_selected = (i == *current_item);
    //     const char* item_text;
    //     if (!items_getter(data, i, &item_text))
    //         item_text = "*Unknown item*";
    //     if (Selectable(item_text, item_selected))
    //     {
    //         value_changed = true;
    //         *current_item = i;
    //     }
    //     if (item_selected)
    //         SetItemDefaultFocus();
    //     PopID();
    // }

    // EndCombo();


	ImGui::PopID();
	if (id != currentSettings->noiseSettings.size() - 1) 
		ImGui::Separator();

}

void		GUIManager::LoadSettings(TerrainSettings * settings) noexcept
{
	currentSettings = settings;
}
