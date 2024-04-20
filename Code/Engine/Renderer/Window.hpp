#pragma once
//--------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <string>

class InputSystem;

struct Vec2;

struct WindowConfig 
{
	InputSystem* m_inputSystem = nullptr;
	std::string m_windowTitle = g_gameConfigBlackboard.GetValue("windowTitle", "XML");
	float m_clientAspect = 2.0f;
	bool m_isFullscreen = false;
	IntVec2 m_windowSize = IntVec2(-1, -1);
	IntVec2 m_windowPosition = IntVec2(-1, -1);
};

class Window {
public:
	Window(WindowConfig const& config);
	~Window();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdowm();


	WindowConfig const& GetConfig()const { return m_config; }
	static Window* GetWindow() { return s_mainWindow; }
	Vec2 GetNormalizedCursorPos()const;
	bool IsWindowFocusd();
	void* GetHwnd() const{return m_windowHandle;};
	IntVec2 GetClientDimensions() const { return m_clientDemision; };
	bool GetXMLPathByOpenFileNameA(std::string& filePath, bool is3D);
	std::string GetXMLFileName(std::string const& directoryPath);
protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	WindowConfig m_config;
	void* m_windowHandle;
	IntVec2 m_clientDemision = IntVec2::ZERO;
	static Window* s_mainWindow;//#ToDo later on: refactor to support multiple windows 
	
};