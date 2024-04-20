#include <Windows.h>
#include "Engine/Renderer/Window.hpp"
#include"Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"

unsigned char const KEYCODE_F1							= VK_F1;
unsigned char const KEYCODE_F2							= VK_F2;
unsigned char const KEYCODE_F3							= VK_F3;
unsigned char const KEYCODE_F4							= VK_F4;
unsigned char const KEYCODE_F5							= VK_F5;
unsigned char const KEYCODE_F6							= VK_F6;
unsigned char const KEYCODE_F7							= VK_F7;
unsigned char const KEYCODE_F8							= VK_F8;
unsigned char const KEYCODE_F9							= VK_F9;
unsigned char const KEYCODE_F10							= VK_F10;
unsigned char const KEYCODE_F11							= VK_F11;
unsigned char const KEYCODE_ESC							= VK_ESCAPE;
unsigned char const KEYCODE_UPARROW						= VK_UP;
unsigned char const KEYCODE_DOWNARROW					= VK_DOWN;
unsigned char const KEYCODE_LEFTARROW					= VK_LEFT;
unsigned char const KEYCODE_RIGHTARROW					= VK_RIGHT;
unsigned char const KEYCODE_LEFT_MOUSE					= VK_LBUTTON;
unsigned char const KEYCODE_RIGHT_MOUSE					= VK_RBUTTON;
unsigned char const KEYCODE_TILDE						= 0xC0; //a.k.a. VK_OEM_3, this is the `(grave)/~(tilde) key
unsigned char const KEYCODE_LEFTBRACKET					= 0xDB; //a.k.a. VK_OEM_4, this is the (/[ key
unsigned char const KEYCODE_ENTER						= VK_RETURN;
unsigned char const KEYCODE_BACKSPACE					= VK_BACK;
unsigned char const KEYCODE_INSERT						= VK_INSERT;
unsigned char const KEYCODE_DELETE						= VK_DELETE;
unsigned char const KEYCODE_HOME						= VK_HOME;
unsigned char const KEYCODE_END							= VK_END;
unsigned char const KEYCODE_SHIFT = VK_SHIFT;
unsigned char const KEYCODE_RIGHTBRACKET = 0xDD;

unsigned char const KEYCODE_COMMA = VK_OEM_COMMA;
unsigned char const KEYCODE_PERIOD = VK_OEM_PERIOD;
unsigned char const KEYCODE_SEMICOLON = VK_OEM_1;
unsigned char const KEYCODE_SINGLEQUETE = VK_OEM_7;

InputSystem* InputSystem::s_theInput = nullptr;


InputSystem::InputSystem(InputSystemConfig const& config)
	:m_config(config)
{
	s_theInput = this;
}

InputSystem::~InputSystem()
{
}

void InputSystem::Startup()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_controllers[i].m_id = i;
	}
	g_eventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_eventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
}

void InputSystem::Shutdown()
{

}

void InputSystem::BeginFrame()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_controllers[i].Update();
	}
	

	if (m_mouseState.m_currentHidden != m_mouseState.m_desiredHidden) 
	{
		m_mouseState.m_currentHidden = m_mouseState.m_desiredHidden;

		if (m_mouseState.m_currentHidden) 
		{
			while (::ShowCursor(false) >= 0) 
			{

			}
		}
		else
		{
			while (::ShowCursor(true) < 0)
			{

			}
		}
	}

	POINT cursorPosi;
	::GetCursorPos(&cursorPosi);
	HWND windowHandle = (HWND)(Window::GetWindow()->GetHwnd());
	::ScreenToClient(windowHandle, &cursorPosi);
	//DebuggerPrintf(" %d %d \n", cursorPosi.x, cursorPosi.y);
	
	if (m_notFirstPersonView) 
	{
		m_mouseState.m_cursorClientPosition = IntVec2((int)cursorPosi.x, (int)cursorPosi.y);
	}
	//POINT clientCursorPosi = cursorPosi;

	RECT clientRect;
	::GetClientRect(windowHandle, &clientRect);
	POINT clientCenter;
	clientCenter.x = (LONG)((clientRect.right - clientRect.left) * (float)0.5f);
	clientCenter.y = (LONG)((clientRect.bottom - clientRect.top) * (float)0.5f);


	if (m_mouseState.m_currentRelative != m_mouseState.m_desiredRelative) 
	{
		m_mouseState.m_currentRelative = m_mouseState.m_desiredRelative;

		m_mouseState.m_cursorClientDelta = IntVec2(0, 0);
		m_mouseState.m_cursorClientPosition = IntVec2((int)clientCenter.x, (int)clientCenter.y);
// 		clientCursorPosi.x = clientCenter.x;
// 		clientCursorPosi.y = clientCenter.y;
		cursorPosi.x = clientCenter.x;
		cursorPosi.y = clientCenter.y;

		::ClientToScreen(windowHandle, &clientCenter);
		::SetCursorPos(clientCenter.x, clientCenter.y);

		//handling the windows problem
		::GetCursorPos(&clientCenter);
		::ScreenToClient(windowHandle, &clientCenter);

		m_mouseState.m_cursorClientPosition = IntVec2((int)clientCenter.x, (int)clientCenter.y);

	}

	if (m_mouseState.m_currentRelative) 
	{
		//Calculate delta
		m_mouseState.m_cursorClientDelta = IntVec2((int)cursorPosi.x - m_mouseState.m_cursorClientPosition.x,
			(int)cursorPosi.y - m_mouseState.m_cursorClientPosition.y);


		//reset the cursor client position 
		::ClientToScreen(windowHandle, &clientCenter);
		::SetCursorPos(clientCenter.x, clientCenter.y);
		::GetCursorPos(&cursorPosi);
		::ScreenToClient(windowHandle, &cursorPosi);

		m_mouseState.m_cursorClientPosition.x = cursorPosi.x;
		m_mouseState.m_cursorClientPosition.y = cursorPosi.y;

	}

}

void InputSystem::EndFrame()
{
	for (int i = 0; i < NUM_KEYCODES; i++)
	{
		m_keyStates[i].m_wasPressedLastFrame = m_keyStates[i].m_IsPressed;
	}
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	if (m_keyStates[keyCode].m_IsPressed && !m_keyStates[keyCode].m_wasPressedLastFrame)
	{
		return true;
	}
	return false;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	if (!m_keyStates[keyCode].m_IsPressed && m_keyStates[keyCode].m_wasPressedLastFrame)
	{
		return true;
	}
	return false;
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	if (m_keyStates[keyCode].m_IsPressed)
	{
		return true;
	}
	return false;
}

void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[(int)keyCode].m_IsPressed = true;
}

void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[(int)keyCode].m_IsPressed = false;
}

XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}


void InputSystem::SetCursorMode(bool hidden, bool relative)
{
	m_mouseState.m_desiredHidden = hidden;
	m_mouseState.m_desiredRelative = relative;
}

Vec2 InputSystem::GetCursorClientDelta() const
{
	return Vec2((float)m_mouseState.m_cursorClientDelta.x, (float)m_mouseState.m_cursorClientDelta.y);
}

Vec2 InputSystem::GetCursorClientPosition() const
{
	return Vec2((float)m_mouseState.m_cursorClientPosition.x, (float)m_mouseState.m_cursorClientPosition.y);
}

Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	POINT cursorCoords;
	::GetCursorPos(&cursorCoords);

	HWND windowHandle = (HWND)(Window::GetWindow()->GetHwnd());
	::ScreenToClient(windowHandle, &cursorCoords);
	RECT clientRect;
	::GetClientRect(windowHandle, &clientRect);
	float mouseClientX = float(cursorCoords.x);
	float mouseClientY = float(cursorCoords.y);
	float clientWidth = float(clientRect.right - clientRect.left);
	float clientHight = float(clientRect.bottom);
	return Vec2(mouseClientX / clientWidth, 1.f - (mouseClientY / clientHight));
}

bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (!s_theInput) 
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	s_theInput->HandleKeyPressed(keyCode);
	return true;
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (!s_theInput) 
	{
		return false; 
	}
	unsigned char keyCode =(unsigned char)args.GetValue("KeyCode", -1);
	s_theInput->HandleKeyReleased(keyCode);
	return true;
}

