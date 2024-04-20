#include "Engine/Renderer/Window.hpp"
#include"Engine/Input/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places



//global and class static variables
Window* Window::s_mainWindow = nullptr;

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* window = Window::GetWindow();
	GUARANTEE_OR_DIE(window != nullptr, "Window::GetWindow() returned null!");
	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE(window != nullptr, "Window::GetConfig().m_inputSystem was null!");


	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			EventArgs args;
			FireEvent("quit");
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			break;
		}
		case WM_CHAR:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("CharInput", args);
			return 0;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			if (input != nullptr)
			{
				input->HandleKeyPressed(keyCode);
				return 0;
			}
			break;
		}
		case WM_RBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			if (input != nullptr)
			{
				input->HandleKeyPressed(keyCode);
				return 0;
			}
			break;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyPressed", args);
			return 0;																						//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			break;
// 			unsigned char keyCode = (unsigned char)wParam;
// 			if (input != nullptr)
// 			{
// 				input->HandleKeyPressed(keyCode);
// 				return 0;
// 			}
// 			break;
		}
		case WM_LBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			if (input != nullptr)
			{
				input->HandleKeyReleased(keyCode);
				return 0;
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			if (input != nullptr)
			{
				input->HandleKeyReleased(keyCode);
				return 0;
			}
			break;
		}
		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyReleased", args);
			return 0;
		}
		break;
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}



Window::Window(WindowConfig const& config)
	:m_config(config)
{
	s_mainWindow = this;
}

Window::~Window()
{

}

void Window::Startup()
{
	//CreateOSWindow(applicationInstanceHandle);
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!
}

void Window::EndFrame()
{

}

void Window::Shutdowm()
{

}


Vec2 Window::GetNormalizedCursorPos() const
{
	POINT cursorCoords;
	::GetCursorPos(&cursorCoords);

	HWND windowHandle = ::GetActiveWindow();
	::ScreenToClient(windowHandle, &cursorCoords);
	RECT clientRect;
	::GetClientRect(windowHandle, &clientRect);
	float mouseClientX = float(cursorCoords.x);
	float mouseClientY = float(cursorCoords.y);
	float clientWidth = float(clientRect.right - clientRect.left);
	float clientHight = float(clientRect.bottom);
	return Vec2(mouseClientX / clientWidth, 1.f - (mouseClientY / clientHight));
}

bool Window::IsWindowFocusd()
{
	return (GetHwnd() == ::GetActiveWindow());
}

//void Window::CreateOSWindow(void* applicationInstanceHandle, float clientAspect)
void Window::CreateOSWindow()
{
	float clientAspect = m_config.m_clientAspect;
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);
	// #SD1ToDo: Add support for full-screen mode (requires different window style flags than windowed mode)
	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	if (m_config.m_isFullscreen) 
	{
		windowStyleFlags = WS_POPUP;
	}
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	


	//float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	//float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);

	float desktopAspect = desktopWidth / desktopHeight;
	// Calculate maximum client size (as some % of desktop size)
	//Change to 1.f comment the if- statement
	//const float maxClientFractionOfDesktop = 0.9f;
	float maxClientFractionOfDesktop = m_config.m_isFullscreen ? 1.f : 0.9f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if (m_config.m_windowSize.x != -1 && m_config.m_windowSize.y != -1) 
	{
		clientWidth = (float)m_config.m_windowSize.x;
		clientHeight = (float)m_config.m_windowSize.y;
	}



	if (!m_config.m_isFullscreen && m_config.m_windowSize.x == -1 && m_config.m_windowSize.y == -1)
	{
		if (clientAspect > desktopAspect)
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * clientAspect;
		}
	}

	

	RECT clientRect;
	if (m_config.m_windowPosition.x != -1 && m_config.m_windowPosition.y != -1) 
	{
		clientRect.left = m_config.m_windowPosition.x;
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.top = m_config.m_windowPosition.y;
		clientRect.bottom = clientRect.top + (int)clientHeight;
	}
	else
	{
		// Calculate client rect bounds by centering the client area
		float clientMarginX = 0.5f * (desktopWidth - clientWidth);
		float clientMarginY = 0.5f * (desktopHeight - clientHeight);

		clientRect.left = (int)clientMarginX;
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.top = (int)clientMarginY;
		clientRect.bottom = clientRect.top + (int)clientHeight;
	}


	// Calculate the outer dimensions of the physical window, including frame
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
	m_windowHandle = hwnd;
	m_clientDemision = IntVec2((int)clientWidth, (int)clientHeight);
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}
		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

//--------------------------------------------------------------------------------------------------
bool Window::GetXMLPathByOpenFileNameA(std::string& filePath, bool is3D)
{
	if (is3D)
	{
		m_config.m_inputSystem->SetCursorMode(true, false);
	}
	OPENFILENAMEA ofn;
	char defaultDirectory[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(MAX_PATH, defaultDirectory);
	char szFileName[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND)GetHwnd();
	ofn.lpstrFilter = "XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select a Model XML File";
	ofn.lpstrInitialDir = defaultDirectory;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		if (strcmp(szFileName, "") != 0)
		{
			filePath = szFileName;
			SetCurrentDirectoryA(defaultDirectory);
			if (is3D)
			{
				m_config.m_inputSystem->SetCursorMode(false, true);
			}
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", 'L'));
			FireEvent("KeyReleased", args);
			return true;
		}
		else
		{
			SetCurrentDirectoryA(defaultDirectory);
			if (is3D)
			{
				m_config.m_inputSystem->SetCursorMode(false, true);
			}
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", 'L'));
			FireEvent("KeyReleased", args);
			return false;
		}
	}
	else
	{
		SetCurrentDirectoryA(defaultDirectory);
		if (is3D)
		{
			m_config.m_inputSystem->SetCursorMode(false, true);
		}
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", 'L'));
		FireEvent("KeyReleased", args);
		return false;
	}
}

//--------------------------------------------------------------------------------------------------
std::string Window::GetXMLFileName(std::string const& directoryPath)
{
	(void)directoryPath;
	// SetCurrentDirectoryA(directoryPath.c_str());
	constexpr int maxfileNameLength = 256;
	char filePath[MAX_PATH] = {};
	OPENFILENAMEA infoAboutUserFileSelection = {};
	infoAboutUserFileSelection.lStructSize = sizeof(OPENFILENAMEA);
	infoAboutUserFileSelection.lpstrFile = filePath;
	infoAboutUserFileSelection.nMaxFile = maxfileNameLength;
	infoAboutUserFileSelection.lpstrInitialDir = directoryPath.c_str();
	infoAboutUserFileSelection.lpstrDefExt = "xml";
	infoAboutUserFileSelection.lpstrFilter = "XML Files\0*.xml\0\0";

	char currentDirectory[MAX_PATH] = {};
	DWORD status = GetCurrentDirectoryA(MAX_PATH, currentDirectory);
	if (status == 0)
	{
		return "INVALID";
	}
	BOOL returnInfo = GetOpenFileNameA(&infoAboutUserFileSelection);
	if (!returnInfo)
	{
		return "INVALID";
	}
	SetCurrentDirectoryA(currentDirectory);

	Strings			backSlashSplittedText = SplitStringOnDelimiter(filePath, '\\');
	size_t			listLength = backSlashSplittedText.size();
	std::string 	fileName = backSlashSplittedText[listLength - 1];
	return fileName;
}