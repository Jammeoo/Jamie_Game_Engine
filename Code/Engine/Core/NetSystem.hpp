#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include <string>


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#ifdef ENABLE_NET

class NetSystem;
extern NetSystem* g_theNetSystem;

struct NetSystemConfig
{
	std::string m_modeString;
	std::string m_hostAddressString;
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
};

class NetSystem
{
public:

	enum class Mode
	{
		NONE = 0,
		CLIENT,
		SERVER,
	};

	enum class ClientState
	{
		INVALID,
		READY_TO_CONNECT,
		CONNECTING,
		CONNECTED,
	};

	enum class ServerState
	{
		INVALID,
		LISTENING,
		CONNECTED,
	};

	NetSystem(const NetSystemConfig& config);
	~NetSystem();

	void Startup();
	void ClientStartup();
	void ServerStartup();
	void Shutdown();
	void ClientShutdown();
	void ServerShutdown();
	void BeginFrame();
	void EndFrame();

	void ClientBeginFrame();
	void ServerBeginFrame();

	static bool Event_RemoteCommand(EventArgs& args);
	static bool Event_BurstTest(EventArgs& args);
	void SendMessageCommand(std::string const& command);
public:
	
public:
	NetSystemConfig m_config;
	Mode m_mode = Mode::NONE;
	

	ClientState m_clientState = ClientState::INVALID;
	ServerState m_serverState = ServerState::INVALID;

	uintptr_t m_clientSocket = ~0ull;
	uintptr_t m_listenSocket = ~0ull;


	unsigned long m_hostAddress = 0;
	unsigned short m_hostPort = 0;

	char *m_sendBuffer = nullptr;
	char *m_recvBuffer = nullptr;

	std::vector<std::string> m_sendQueue;
	std::string m_recvRemaining;
};

#endif
