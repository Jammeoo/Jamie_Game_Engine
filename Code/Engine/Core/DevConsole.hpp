#pragma once
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <mutex>
#include <vector>
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Renderer;
class Camera;
class BitmapFont;
class StopWatch;
struct AABB2;

class DevConsole;

extern DevConsole* g_theDevConsole;

struct DevConsoleLine 
{
	Rgba8 m_color;
	std::string m_text;
};

struct DevConsoleConfig {

	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	std::string m_fortName = "SquirrelFixedFont";
	float m_fontAspect = 0.7f;
	float m_linesOnScreen = 40.0f;
	int m_maxCommandHistory = 128;
};

class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds);
	void ToggleOpen();
	
	static const Rgba8 ERROR_;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 COMMAND_ECHO;
	static const Rgba8 INPUT_TEXT;
	static const Rgba8 INPUT_CARET;

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);

	void DrawBlackQuad(AABB2 const& bounds)const;
	bool GetIsOpened()const;
protected:
	DevConsoleConfig m_config;
	bool m_isOpen = false;					
	std::vector<DevConsoleLine> m_lines;
	std::string m_inputText;
	int m_caretPosition = 0;
	bool m_caretVisible = true;
	StopWatch* m_caretStopwatch;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = -1;


	std::string m_fontFilePath;

	std::mutex m_devConsole;
};