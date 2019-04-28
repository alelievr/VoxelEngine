#pragma once

#include <iostream>
#include <string>
#include "TerrainSettings.hpp"
#include "LWGC.hpp"

using namespace LWGC;

class		GUIManager
{
	private:
		static TerrainSettings *	currentSettings;
		static GameObject *			parent;

	public:
		GUIManager(void) = delete;
		GUIManager(const GUIManager &) = delete;
		virtual ~GUIManager(void) = delete;

		GUIManager &	operator=(GUIManager const & src) = delete;

		static void		Initialize(void) noexcept;
		static void		LoadSettings(TerrainSettings * settings) noexcept;
		static void		NoiseSettingsPanel();
};
