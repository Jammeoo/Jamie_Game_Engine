#pragma once
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/JobSystem.hpp"

#define UNUSED(x) (void)(x)
#define STATIC
#pragma warning(disable: 26812)
#pragma warning(disable: 4127)


//#define ENABLE_NET
//#define ENABLE_D3D12
//#define USE_PIX
extern NamedStrings g_gameConfigBlackboard;
extern JobSystem* g_theJobSystem;

