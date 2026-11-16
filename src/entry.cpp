///----------------------------------------------------------------------------------------------------
/// Name         :  entry.cpp
/// Description  :  GW2 FPS Unlocker addon for Nexus with ImGui 1.80 UI.
///----------------------------------------------------------------------------------------------------

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "nexus/Nexus.h"
#include "imgui/imgui.h"
#include "FpsPatch.h"

/* Prototypes */
void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void RenderOptions();
void RenderSettingsWnd();

/* Globals */
AddonDefinition_t AddonDef = {};
HMODULE           hSelf = nullptr;
AddonAPI_t* APIDefs = nullptr;

/* UI State */
bool              g_ShowWindow = false;
bool              g_IsEnabled = false;
int               g_TargetFPS = 240;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: hSelf = hModule; break;
	case DLL_PROCESS_DETACH: break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
	AddonDef.Signature = (uint32_t)-48217;
	AddonDef.APIVersion = NEXUS_API_VERSION;
	AddonDef.Name = "FPS Unlocker";
	AddonDef.Version.Major = 2026;
	AddonDef.Version.Minor = 11;
	AddonDef.Version.Build = 16;
	AddonDef.Version.Revision = 0;
	AddonDef.Author = "Nekonekoo";
	AddonDef.Description = "Unlocks the 240 FPS cap by patching the game's frametime setter, with an ImGui UI.";
	AddonDef.Load = AddonLoad;
	AddonDef.Unload = AddonUnload;
	AddonDef.Flags = AF_None;
	AddonDef.Provider = UP_Raidcore;
	AddonDef.UpdateLink = nullptr;

	return &AddonDef;
}

static void LogPatch(const char* msg)
{
	if (APIDefs) {
		APIDefs->Log(LOGL_INFO, "FPS Unlocker", msg);
	}
}

/* Settings */
static std::filesystem::path SettingsPath()
{
	const char* dir = APIDefs ? APIDefs->Paths_GetAddonDirectory("FPSUnlocker") : nullptr;
	return dir ? std::filesystem::path(dir) / "settings.json" : std::filesystem::path();
}

static std::string JsonValue(const std::string& json, const char* key)
{
	const std::string needle = std::string("\"") + key + "\"";
	size_t pos = json.find(needle);
	if (pos == std::string::npos) return {};
	pos = json.find(':', pos + needle.size());
	if (pos == std::string::npos) return {};

	pos = json.find_first_not_of(" \t\r\n", pos + 1);
	if (pos == std::string::npos) return {};
	const size_t end = json.find_first_of(",}\r\n", pos);
	std::string val = json.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
	while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
	return val;
}

static void LoadSettings()
{
	const auto path = SettingsPath();
	if (path.empty()) return;

	std::ifstream file(path);
	if (!file) return;
	const std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	const std::string enabled = JsonValue(json, "enabled");
	const std::string fps     = JsonValue(json, "targetFps");
	const std::string window  = JsonValue(json, "showWindow");

	if (!enabled.empty()) g_IsEnabled  = enabled == "true";
	if (!window.empty())  g_ShowWindow = window == "true";
	if (!fps.empty())
	{
		try { g_TargetFPS = std::stoi(fps); } catch (...) {}
	}

	if (g_TargetFPS < 30)    g_TargetFPS = 30;
	if (g_TargetFPS > 10000) g_TargetFPS = 10000;
}

static void SaveSettings()
{
	const auto path = SettingsPath();
	if (path.empty()) return;

	std::error_code ec;
	std::filesystem::create_directories(path.parent_path(), ec);

	std::ofstream file(path, std::ios::trunc);
	file << "{\n"
	     << "  \"enabled\": "    << (g_IsEnabled ? "true" : "false") << ",\n"
	     << "  \"targetFps\": "  << g_TargetFPS << ",\n"
	     << "  \"showWindow\": " << (g_ShowWindow ? "true" : "false") << "\n"
	     << "}\n";
}

static void SetTargetFps(int fps)
{
	g_TargetFPS = fps;
	FpsPatch::Apply(g_TargetFPS, LogPatch);
	SaveSettings();
}

void DrawUnlockerControls()
{
	if (FpsPatch::IsUnsupported())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "This game build isn't supported yet.");
		ImGui::TextWrapped("A game update changed the code this addon patches. "
			"The game runs normally with its default FPS limit. Please wait for an addon update.");
		return;
	}

	if (ImGui::Checkbox("Enable FPS Unlocker", &g_IsEnabled))
	{
		if (g_IsEnabled) {
			FpsPatch::Apply(g_TargetFPS, LogPatch);
		}
		else {
			FpsPatch::Revert(LogPatch);
		}
		SaveSettings();
	}

	if (g_IsEnabled)
	{
		ImGui::SliderInt("Target Max FPS", &g_TargetFPS, 30, 10000, "%d FPS");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			SetTargetFps(g_TargetFPS);
		}

		const int presets[] = { 60, 144, 240, 360, 500, 1000 };
		for (int i = 0; i < (int)(sizeof(presets) / sizeof(presets[0])); i++)
		{
			if (i > 0) ImGui::SameLine();
			const std::string label = std::to_string(presets[i]) + " FPS";
			if (ImGui::Button(label.c_str())) { SetTargetFps(presets[i]); }
		}

		if (FpsPatch::Divisor() == 1000)
		{
			const int effective = FpsPatch::EffectiveFps(g_TargetFPS);
			if (effective == 0)
				ImGui::TextDisabled("This game build uses ms frametimes: effectively uncapped.");
			else if (effective != g_TargetFPS)
				ImGui::TextDisabled("This game build uses ms frametimes: real cap ~%d FPS.", effective);
		}
	}

	ImGui::Separator();
	ImGui::Text("Status: %s", FpsPatch::IsPatched() ? "Active" : "Inactive");
	ImGui::TextDisabled("Tip: Use Fullscreen mode if Windows DWM caps borderless to monitor Hz.");
}

void RenderOptions()
{
	DrawUnlockerControls();
	ImGui::Spacing();
	if (ImGui::Checkbox("Show Floating Window", &g_ShowWindow)) {
		SaveSettings();
	}
}

void RenderSettingsWnd()
{
	if (!g_ShowWindow) return;

	ImGui::SetNextWindowSize(ImVec2(380, 170), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("FPS Unlocker##FloatingWnd", &g_ShowWindow, ImGuiWindowFlags_NoCollapse))
	{
		DrawUnlockerControls();
	}
	ImGui::End();
}

void AddonLoad(AddonAPI_t* aApi)
{
	APIDefs = aApi;

	// Synchronize ImGui context with Nexus core using definitions from Nexus.h
	ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
	ImGui::SetAllocatorFunctions(
		(void* (*)(size_t, void*))APIDefs->ImguiMalloc,
		(void (*)(void*, void*))APIDefs->ImguiFree
	);

	// Register GUI render callbacks using official Nexus API pointers
	APIDefs->GUI_Register(RT_Render, RenderSettingsWnd);
	APIDefs->GUI_Register(RT_OptionsRender, RenderOptions);

	LoadSettings();

	FpsPatch::Init(LogPatch);

	// Apply patch on load if enabled
	if (g_IsEnabled) {
		FpsPatch::Apply(g_TargetFPS, LogPatch);
	}
}

void AddonUnload()
{
	if (APIDefs) {
		APIDefs->GUI_Deregister(RenderSettingsWnd);
		APIDefs->GUI_Deregister(RenderOptions);
	}

	FpsPatch::Revert(LogPatch);
}