///----------------------------------------------------------------------------------------------------
/// Name         :  entry.cpp
/// Description  :  GW2 FPS Unlocker addon for Nexus with ImGui 1.80 UI.
///----------------------------------------------------------------------------------------------------

#include <Windows.h>
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
bool              g_IsEnabled = true;
int               g_TargetFPS = 1000;

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
	AddonDef.Version.Major = 0;
	AddonDef.Version.Minor = 8;
	AddonDef.Version.Build = 0;
	AddonDef.Version.Revision = 0;
	AddonDef.Author = "Nekonekoo";
	AddonDef.Description = "Unlocks the 240 FPS cap with NOP patching and an ImGui UI.";
	AddonDef.Load = AddonLoad;
	AddonDef.Unload = AddonUnload;
	AddonDef.Flags = AF_None;

	return &AddonDef;
}

static void LogPatch(const char* msg)
{
	if (APIDefs) {
		APIDefs->Log(LOGL_INFO, "FPS Unlocker", msg);
	}
}

void DrawUnlockerControls()
{
	if (ImGui::Checkbox("Enable FPS Unlocker", &g_IsEnabled))
	{
		if (g_IsEnabled) {
			FpsPatch::Apply(g_TargetFPS, LogPatch);
		}
		else {
			FpsPatch::Revert(LogPatch);
		}
	}

	if (g_IsEnabled)
	{
		if (ImGui::SliderInt("Target Max FPS", &g_TargetFPS, 30, 10000, "%d FPS"))
		{
			FpsPatch::Apply(g_TargetFPS, LogPatch);
		}

		if (ImGui::Button("240 FPS")) { g_TargetFPS = 240;  FpsPatch::Apply(g_TargetFPS, LogPatch); }
		ImGui::SameLine();
		if (ImGui::Button("360 FPS")) { g_TargetFPS = 360;  FpsPatch::Apply(g_TargetFPS, LogPatch); }
		ImGui::SameLine();
		if (ImGui::Button("500 FPS")) { g_TargetFPS = 500;  FpsPatch::Apply(g_TargetFPS, LogPatch); }
		ImGui::SameLine();
		if (ImGui::Button("1000 FPS")) { g_TargetFPS = 1000; FpsPatch::Apply(g_TargetFPS, LogPatch); }
	}

	ImGui::Separator();
	ImGui::Text("Status: %s", FpsPatch::IsPatched() ? "Active" : "Inactive");
	ImGui::TextDisabled("Tip: Use Fullscreen mode if Windows DWM caps borderless to monitor Hz.");
}

void RenderOptions()
{
	DrawUnlockerControls();
	ImGui::Spacing();
	ImGui::Checkbox("Show Floating Window", &g_ShowWindow);
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