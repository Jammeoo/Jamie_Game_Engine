#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"

const Rgba8 DevConsole::ERROR_ = Rgba8(255, 0, 0, 255);
const Rgba8 DevConsole::WARNING = Rgba8(255, 0, 102, 255);
const Rgba8 DevConsole::INFO_MAJOR = Rgba8(0, 255, 0, 255);
const Rgba8 DevConsole::INFO_MINOR = Rgba8(0, 255, 255, 255);
const Rgba8 DevConsole::COMMAND_ECHO = Rgba8(163, 41, 122, 255);
const Rgba8 DevConsole::INPUT_TEXT = Rgba8(255, 255, 153, 255);
const Rgba8 DevConsole::INPUT_CARET = Rgba8(255, 255, 255, 255);

DevConsole::DevConsole(DevConsoleConfig const& config):m_config(config)
{
	m_fontFilePath = "Data/Images/" + m_config.m_fortName;
	StopWatch* stopwatch = new StopWatch(0.5f);
	m_caretStopwatch = stopwatch;
	
}

DevConsole::~DevConsole()
{

}

void DevConsole::Startup()
{
	g_eventSystem->SubscribeEventCallbackFunction("CharInput", DevConsole::Event_CharInput);
	g_eventSystem->SubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
	g_eventSystem->SubscribeEventCallbackFunction("help", DevConsole::Command_Help);
	g_eventSystem->SubscribeEventCallbackFunction("clear", DevConsole::Command_Clear);
	AddLine(DevConsole::INFO_MINOR, "Type help for a list of commands");
}

void DevConsole::Shutdown()
{
	g_eventSystem->UnsubscribeFromAllEvents();
}

void DevConsole::BeginFrame()
{
	//DebuggerPrintf("time=%f %f\n ", m_caretStopwatch->m_clock->GetTotalSeconds(), GetCurrentTimeSeconds());
	if (m_caretStopwatch->IsStopped())
	{
		m_caretStopwatch->Start();
	}
	if (m_caretStopwatch->DecrementDurationIfElapsed()) 
	{
		m_caretVisible = !m_caretVisible;
		//DebuggerPrintf("Toggle\n ");
	}

}

void DevConsole::EndFrame()
{

}

void DevConsole::Execute(std::string const& consoleCommandText) 
{
	if (consoleCommandText.empty()) 
	{
		ToggleOpen();
		return;
	}
	Strings splitedConmmandText = SplitStringWithQuotes(consoleCommandText, ' ');
	EventArgs args;
	//if (splitedConmmandText[0] == "Server") 
	//{
	//	AddLine(COMMAND_ECHO, consoleCommandText);
	//	return;
	//}
	std::vector<std::string> vectorOfStrings;
	g_eventSystem->GetNamesOfAllEvents(vectorOfStrings);

	if (std::find(vectorOfStrings.begin(), vectorOfStrings.end(), splitedConmmandText[0]) == vectorOfStrings.end())
	{
		std::string tempErrorString = "Unknown Command " + splitedConmmandText[0];
		AddLine(ERROR_, tempErrorString);
		return;
	}
	if ((int)splitedConmmandText.size() == 2)
	{
		for (int i = 1; i < splitedConmmandText.size(); i++)
		{
			Strings doubleSplitedCommand = SplitStringWithQuotes(splitedConmmandText[i], '=');

			if ((int)doubleSplitedCommand.size() == 2)
			{
				args.SetValue(doubleSplitedCommand[0], doubleSplitedCommand[1]);
			}
		}
	}
	FireEvent(splitedConmmandText[0], args);
	//Echo command part:
	if (splitedConmmandText[0] == "Echo") 
	{
		std::string tempString = splitedConmmandText[1];
		Strings echoCommand = SplitStringOnDelimiter(tempString, '=');
		if ((int)echoCommand.size() > 1) 
		{
			AddLine(COMMAND_ECHO, echoCommand[1]);									//delicate
		}
	}
	else
	{
		AddLine(INFO_MAJOR, consoleCommandText);
	}
	
}


void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	m_devConsole.lock();
	DevConsoleLine devConsoleLine;
	devConsoleLine.m_color = color;
	devConsoleLine.m_text = text;
	m_lines.push_back(devConsoleLine);
	m_devConsole.unlock();
}

void DevConsole::Render(AABB2 const& bounds)
{
	if (!m_isOpen) return;

	m_devConsole.lock();
	m_config.m_renderer->BeginCamera(*m_config.m_camera);
	DrawBlackQuad(bounds);
	BitmapFont* font = m_config.m_renderer->CreateOrGetBitmapFont(m_fontFilePath.c_str());
	float cellHeight = bounds.GetDimensions().y / m_config.m_linesOnScreen;
	float cellWidth = cellHeight / m_config.m_fontAspect;
	std::vector<Vertex_PCU> fontVertex;
	fontVertex.reserve(90000);

	Vec2 textBoxMins = bounds.m_mins + Vec2(0.f, cellHeight);

	int lineNum = (int)m_config.m_linesOnScreen - 1;
	AABB2 shownBounds = AABB2(bounds);
	shownBounds.m_mins.y += cellHeight;
	for (int lineIndex = (int)m_lines.size() - 1; lineIndex >= 0 && lineNum <= 40; lineIndex-- && lineNum--) 
	{
		shownBounds.m_mins.y = (m_lines.size() - lineIndex ) * cellHeight;
		shownBounds.m_maxs.y = shownBounds.m_mins.y + cellHeight;

		font->AddVertsForTextInBox2D(fontVertex, shownBounds, cellHeight, m_lines[lineIndex].m_text, m_lines[lineIndex].m_color,
			m_config.m_fontAspect,  Vec2(0.f, 0.f), TextDrawMode::OVERRUN, 99999999);
		if (lineIndex < (int)m_lines.size() - 1 - (int)m_config.m_linesOnScreen)
		{
			break;
		}
	}

	font->AddVertsForTextInBox2D(fontVertex, bounds, cellHeight, m_inputText, INPUT_TEXT, m_config.m_fontAspect,
		Vec2(0.f, 0.f), TextDrawMode::OVERRUN, 99999999);
	m_config.m_renderer->SetModelConstants();
	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->BindTexture(&font->GetTexture());
	m_config.m_renderer->BindShader(nullptr);
	m_config.m_renderer->DrawVertexArray((int)fontVertex.size(), fontVertex.data());
	//change the box's m_mins, 
	

	std::vector<Vertex_PCU> caretVertex;
	caretVertex.reserve(50);
	float widthPerCharAlign = 0.254f / cellWidth;
	float caretPosiOffset = 0.1f * widthPerCharAlign;

	Vec2 adjustedCaretAlignment = Vec2(m_caretPosition * widthPerCharAlign - caretPosiOffset, 0.f);
	font->AddVertsForTextInBox2D(caretVertex, bounds, cellHeight, "|", DevConsole::INPUT_CARET,
			m_config.m_fontAspect, adjustedCaretAlignment, TextDrawMode::OVERRUN, 99999999);
	if (m_caretVisible) 
	{
		m_config.m_renderer->SetModelConstants();
		m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_config.m_renderer->BindTexture(&font->GetTexture());
		m_config.m_renderer->DrawVertexArray((int)caretVertex.size(), caretVertex.data());
	}
	m_config.m_renderer->BindTexture(nullptr);
	m_config.m_renderer->EndCamera(*m_config.m_camera);
	m_devConsole.unlock();

}

void DevConsole::ToggleOpen()
{
	m_isOpen = !m_isOpen;
	if(m_isOpen) m_caretStopwatch->Start();
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	if (!g_theDevConsole) 
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	if (keyCode == KEYCODE_TILDE) 
	{
		g_theDevConsole->ToggleOpen();

		return true;
	}
	if (g_theDevConsole->m_isOpen) 
	{
		if(keyCode==KEYCODE_ENTER){ //Execute / input text/ clear(input text)/
			g_theDevConsole->Execute(g_theDevConsole->m_inputText);
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_caretPosition = 0;
			return true;
		}
		if (keyCode == KEYCODE_ESC) 
		{
			if (!g_theDevConsole->m_inputText.empty()) 
			{
				g_theDevConsole->m_inputText.clear();
				g_theDevConsole->m_caretPosition = 0;
			}
			else 
			{
				g_theDevConsole->m_isOpen = false;
			}
			return true;
		}
		if (keyCode == KEYCODE_LEFTARROW) 
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == 0) 
			{
				return true;
			}
			g_theDevConsole->m_caretPosition--;
			return true;
		}
		if (keyCode == KEYCODE_RIGHTARROW)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == (int)g_theDevConsole->m_inputText.length()) //edge of window
			{
				return true;
			}
			g_theDevConsole->m_caretPosition++;
			return true;
		}
		if (keyCode == KEYCODE_BACKSPACE) 
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			
			if (g_theDevConsole->m_caretPosition <= 0) 
			{
				return true;
			}
			else 
			{
				std::string subString_first = std::string(g_theDevConsole->m_inputText, 0, g_theDevConsole->m_caretPosition - (size_t)1);
				std::string subString_second = std::string(g_theDevConsole->m_inputText, g_theDevConsole->m_caretPosition, g_theDevConsole->m_inputText.size() - 1);
				--g_theDevConsole->m_caretPosition;

				g_theDevConsole->m_inputText = subString_first + subString_second;
			}
			return true;
		}
		if (keyCode == KEYCODE_DELETE)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == (int)g_theDevConsole->m_inputText.length()) //edge of window
			{
				return true;
			}
			std::string subString_first = std::string(g_theDevConsole->m_inputText, 0, g_theDevConsole->m_caretPosition);
			std::string subString_second = std::string(g_theDevConsole->m_inputText, g_theDevConsole->m_caretPosition + 1, g_theDevConsole->m_inputText.size() - 1);
			g_theDevConsole->m_inputText = subString_first + subString_second;

			return true;
		}
		if (keyCode == KEYCODE_HOME) 
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			g_theDevConsole->m_caretPosition = 0;
			return true;
		}
		if (keyCode == KEYCODE_END) 
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.length();
			return true;
		}
		return true;

	}
	return false;
}

bool DevConsole::Event_CharInput(EventArgs& args)
{
	// Valid characters have a code of >=32 and <= 126 and not '~' or '`'.
	if (!g_theDevConsole)
	{
		return false;
	}
	if (g_theDevConsole->m_isOpen) 
	{
		unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
		//g_theDevConsole->m_inputText += keyCode;
		if ((keyCode >= 32 && keyCode <= 126) && (keyCode != '`' && keyCode != KEYCODE_TILDE)) 
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			g_theDevConsole->m_inputText.insert(g_theDevConsole->m_caretPosition, 1, keyCode);
			g_theDevConsole->m_caretPosition++;
			return true;
		}
	}

	return false;
}


bool DevConsole::Command_Clear(EventArgs& args)
{
	UNUSED(args);

	if (g_theDevConsole == nullptr) 
	{
		return false;
	}

	g_theDevConsole->m_lines.clear();
	return true;
}

bool DevConsole::Command_Help(EventArgs& args)
{
	UNUSED(args);
	if (g_theDevConsole == nullptr)
	{
		return false;
	}
	Strings nameVector;
	g_eventSystem->GetNamesOfAllEvents(nameVector);
	for (int i = 0; i < (int)nameVector.size(); i++) 
	{
		g_theDevConsole->AddLine(INFO_MINOR, nameVector[i].c_str());
	}
	return true;
}

void DevConsole::DrawBlackQuad(AABB2 const& bounds) const
{
	std::vector<Vertex_PCU> blackQuad;
	AddVertsForAABB2D(blackQuad, bounds, Rgba8(0, 0, 0, 150));
	m_config.m_renderer->SetModelConstants();
	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->BindTexture(nullptr);
	m_config.m_renderer->DrawVertexArray((int)blackQuad.size(), blackQuad.data());
	m_config.m_renderer->BindTexture(nullptr);
}

bool DevConsole::GetIsOpened() const
{
	return m_isOpen;
}
