#include "ini.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <windows.h>

#include <imgui.h>
#include <imgui_internal.h>

Settings gSettings;

static void* ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
	return &gSettings;
}

static void ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
	float x, y, z;
	int i, j;
	if (sscanf(line, "Color=%f,%f,%f", &x, &y, &z) == 3) {
		gSettings.color[0] = x;
		gSettings.color[1] = y;
		gSettings.color[2] = z;
	} else if (sscanf(line, "Zoom=%i", &i) == 1) {
		gSettings.zoomLevel = i;
	} else if (sscanf(line, "Theme=%i", &i) == 1){
		gSettings.theme = i;
	} else if (sscanf(line, "FontSize=%f", &x) == 1){
		gSettings.fontSize = x;
	} else if (sscanf(line, "pos=%i,%i", &i, &j) == 1){
		gSettings.posX = i;
		gSettings.posY = j;
	} else if (sscanf(line, "size=%i,%i", &i, &j) == 1){
		gSettings.winSizeX = i;
		gSettings.winSizeY = j;
	} else if (sscanf(line, "Maximized=%i", &i) == 1){
		gSettings.maximized = i;
	}
}

static void Write(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
	buf->appendf("[%s][]\n", handler->TypeName);
	buf->appendf("Color=%f,%f,%f\n", gSettings.color[0], gSettings.color[1], gSettings.color[2]);
	buf->appendf("Zoom=%i\n", gSettings.zoomLevel);
	buf->appendf("Theme=%i\n", gSettings.theme);
	buf->appendf("FontSize=%f\n", gSettings.fontSize);
	buf->appendf("pos=%hi,%hi\n", gSettings.posX, gSettings.posY);
	buf->appendf("size=%hi,%hi\n", gSettings.winSizeX, gSettings.winSizeY);
	buf->appendf("Maximized=%i\n", gSettings.maximized);
	buf->append("\n");
}

void InitIni()
{
	ImGuiContext &context = *ImGui::GetCurrentContext();
	ImGuiSettingsHandler ini_handler{};
	ini_handler.TypeName = "Other settings";
	ini_handler.TypeHash = ImHashStr("Other settings");
	ini_handler.ReadOpenFn = ReadOpen;
	ini_handler.ReadLineFn = ReadLine;
	ini_handler.WriteAllFn = Write;
	context.SettingsHandlers.push_back(ini_handler);
	ImGui::LoadIniSettingsFromDisk(context.IO.IniFilename);
}
