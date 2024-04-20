#include "Engine/Core/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

//#if !defined( ENGINE_DISABLE_NETWORK )
#ifdef ENABLE_NET

NetSystem* g_theNetSystem = nullptr;
NetSystem::NetSystem(const NetSystemConfig& config) : m_config(config)
{
	if (m_config.m_modeString == "Client") 
	{
		m_mode = Mode::CLIENT;
		const int sendBufferSize = m_config.m_sendBufferSize;
		m_sendBuffer = new char[sendBufferSize];
		const int recvbufferSize = m_config.m_recvBufferSize;
		m_recvBuffer = new char[recvbufferSize];
	}
	else if(m_config.m_modeString == "Server")
	{
		m_mode = Mode::SERVER;
		const int sendBufferSize = m_config.m_sendBufferSize;
		m_sendBuffer = new char[sendBufferSize];
		const int recvbufferSize = m_config.m_recvBufferSize;
		m_recvBuffer = new char[recvbufferSize];
	}
	else
	{
		m_mode = Mode::NONE;
	}

}

NetSystem::~NetSystem()
{
	delete m_sendBuffer;
	delete m_recvBuffer;
	m_sendQueue.clear();
	m_recvRemaining.clear();
	
}

void NetSystem::Startup()
{
	//as a client
	if (m_mode == Mode::CLIENT) 
	{
		ClientStartup();
	}
	//as a server
	if (m_mode == Mode::SERVER)
	{
		ServerStartup();
	}
	if (m_mode != Mode::NONE) 
	{
		g_eventSystem->SubscribeEventCallbackFunction("RemoteCommand", Event_RemoteCommand);
		g_eventSystem->SubscribeEventCallbackFunction("BurstTest", Event_BurstTest);
	}
	//std::string text1 = "Echo Message=Jamie";
	//std::string text2 = "Finished";
	//std::string text3 = "Network";
	//std::string text4 = "Friday";
	//m_sendQueue.push_back(text1);

}
void NetSystem::ClientStartup()
{
	// Startup Windows sockets.
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);

	// Create client socket.
	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Set blocking mode.
	unsigned long blockingMode = 1;
	result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
	if (result == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Error happened after create socket\n");
	}

	m_clientState = ClientState::READY_TO_CONNECT;// prepared ready!
	DebuggerPrintf("Client READY TO CONNECT\n");
	//Get host address from string.
	IN_ADDR addr;
	//Divide m_hostAddressString in to two parts;
	std::string rawString = m_config.m_hostAddressString;
	Strings addrAndPort = SplitStringOnDelimiter(rawString, ':');

	GUARANTEE_OR_DIE(((int)addrAndPort.size() == 2), "Error: more stuff came out after SplitStringOnDelimiter()");

	std::string addrString = addrAndPort[0];	//#ToDo: get this from somewhere else

	PCSTR oldString = (PCSTR)addrString.c_str();
	result = inet_pton(AF_INET, oldString, &addr);
	m_hostAddress = ntohl(addr.S_un.S_addr);


	std::string hostPortString = addrAndPort[1];
	//Get host port from string.
	m_hostPort = (unsigned short)(atoi(hostPortString.c_str()));

}

void NetSystem::ServerStartup() 
{
	//Startup Windows sockets.
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Error happened after SERVER startup windows socket\n");
	}
	// Create listen socket
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Set blocking mode.
	unsigned long blockingMode = 1;
	result = ioctlsocket(m_listenSocket, FIONBIO, &blockingMode);
	if (result == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Error happened after set blocking mode\n");
	}

	DebuggerPrintf("Server READY TO CONNECT\n");

	//Divide m_hostAddressString in to two parts;
	std::string rawString = m_config.m_hostAddressString;
	Strings addrAndPort = SplitStringOnDelimiter(rawString, ':');
	GUARANTEE_OR_DIE(((int)addrAndPort.size() == 2), "Error: more stuff came out after SplitStringOnDelimiter()");
	std::string hostPortString = addrAndPort[1];

	//Get host port from string.
	m_hostAddress = INADDR_ANY;
	m_hostPort = (unsigned short)(atoi(hostPortString.c_str()));

	//Bind the listen socket to a port.
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
	addr.sin_port = htons(m_hostPort);
	result = bind(m_listenSocket, (sockaddr*)&addr, (int)sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Error happened after Binding the listen socket to a port\n");
	}
	//Listen for connections to accept.
	result = listen(m_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Error happened when Listening for connections to accept\n");
	}
	m_serverState = ServerState::LISTENING;
}
void NetSystem::Shutdown()
{
	if (m_mode == Mode::CLIENT) 
	{
		// Close all open sockets.
		closesocket(m_clientSocket);

		// Shutdown Windows sockets.
		WSACleanup();

	}
	if (m_mode == Mode::SERVER)
	{
		//Close all open sockets.
		closesocket(m_clientSocket);
		closesocket(m_listenSocket);


		// Shutdown Windows sockets.
		WSACleanup();

	}
}

void NetSystem::BeginFrame()
{
	//Both client and server should have a send buffer

	//as a client
	if (m_mode == Mode::CLIENT)
	{
		ClientBeginFrame();
	}
	if (m_mode == Mode::SERVER)
	{
		ServerBeginFrame();
	}

}

void NetSystem::ClientBeginFrame() 
{
	// Attempt to connect if we haven't already.
	if (m_clientState == ClientState::READY_TO_CONNECT)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
		addr.sin_port = htons(m_hostPort);
		int result = connect(m_clientSocket, (sockaddr*)(&addr), (int)sizeof(addr));//In school bus

		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE("Error happened at Connect()\n");
			}
			else
			{
				m_clientState = ClientState::CONNECTING;
				return;
			}
		}
	}

	if (m_clientState == ClientState::CONNECTING)
	{
		//Check if our connection attempt failed.
		fd_set failedSockets;
		FD_ZERO(&failedSockets);
		FD_SET(m_clientSocket, &failedSockets);
		timeval failedWaitTime = { };
		int failedResult = select(0, NULL, NULL, &failedSockets, &failedWaitTime);

		if (failedResult == SOCKET_ERROR)
		{
			ERROR_AND_DIE("Error happened at Select()1\n");
		}
		if(failedResult > 0 && FD_ISSET(m_clientSocket, &failedSockets))
		{
			m_clientState = ClientState::READY_TO_CONNECT;
			return;
		}

		//Check if our connection attempt completed.
		fd_set successSockets;
		FD_ZERO(&successSockets);
		FD_SET(m_clientSocket, &successSockets);
		timeval successWaitTime = { };
		int successResult = select(0, NULL, &successSockets, NULL, &successWaitTime);
		if (successResult == SOCKET_ERROR) 
		{
			ERROR_AND_DIE("Error happened at Select()2\n")
		}
		//We are connected if the following is true.
		if (successResult > 0 && FD_ISSET(m_clientSocket, &successSockets))
		{
			m_clientState = ClientState::CONNECTED;
			DebuggerPrintf("Client is CONNECTED!!!\n");
		}
	}

	if (m_clientState == ClientState::CONNECTED)
	{
		//Send and receive if we are connected.


		int receiveResult = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);		
		if (receiveResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAECONNRESET)
			{
				m_clientState = ClientState::READY_TO_CONNECT;
				Shutdown();
				Startup();
				return;
			}
			else if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE("Error happened at Recv()\n");
			}
			// Else, we are in the middle of receiving a message
		}
		else if (receiveResult == 0)
		{
			// 0 means connection was broken while nothing is being received, (cleanly broken)
			m_clientState = ClientState::READY_TO_CONNECT;
			Shutdown();
			Startup();
			return;
		}
		DebuggerPrintf("Client is in receiving\n");

		for (int i = 0; i < (int)m_sendQueue.size(); ++i)
		{
			if (m_config.m_sendBufferSize < sizeof(m_sendQueue[i])) 
			{
				ERROR_AND_DIE("The Size of Send Buffer is Too Small");
			}
			if (!strncpy_s(m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen(m_sendQueue[i].c_str()) + 1) == 0)
				//Copy failed if true
			{
				ERROR_AND_DIE("This copy action is failed");
			}

			int sendResult = send(m_clientSocket, m_sendBuffer, (int)strlen(m_sendBuffer) + 1, 0);
			if (sendResult == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					if (error == WSAECONNRESET/*10054*/) //An existing connection was forcibly closed by the remote host.
					{
						m_sendQueue.clear();
						m_clientState = ClientState::READY_TO_CONNECT;
						Shutdown();
						Startup();
					}
					else 
					{
						ERROR_AND_DIE("Error happened at send()\n");
					}
					
				}
			}
		}

		
		m_sendQueue.clear();
		for (int recvCharIndex = 0; recvCharIndex < receiveResult; ++recvCharIndex)
		{
			char tempChar = m_recvBuffer[recvCharIndex];
			if (tempChar > 0)
			{
				m_recvRemaining += tempChar;
			}
			else
			{
				if (!m_recvRemaining.empty())
				{
					g_theDevConsole->Execute(m_recvRemaining);
					m_recvRemaining.clear();
				}
				DebuggerPrintf("Client receive something to server\n");
				continue;
			}
		}
		return;
	}
}

void NetSystem::ServerBeginFrame() 
{

	if (m_serverState == ServerState::LISTENING)
	{
		//If we do not have a connection, check for connections to accept.

		uintptr_t socket = accept(m_listenSocket, NULL, NULL);
		if (socket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE("Error happened at Server accept()\n");
			}
			else
			{
				DebuggerPrintf("No connection for Server to accept\n");
				return;
			}
		}
		else
		{
			m_clientSocket = socket;
			//If a connection is accepted set blocking mode.
			unsigned long blockingMode = 1;
			int result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				DebuggerPrintf("The error is %d\n", error);
				ERROR_AND_DIE("Server Can not set blocking mode\n");
			}
			else
			{
				m_serverState = ServerState::CONNECTED;
				DebuggerPrintf("Server accepts a connection and set block mode successfully\n");
			}
		}
	}
	if (m_serverState == ServerState::CONNECTED)
	{
		//If we have a connection, send and receive on our client socket.
		int receiveResult = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);
		if (receiveResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAECONNRESET)
			{
				m_serverState = ServerState::LISTENING;
				Shutdown();
				Startup();
				return;
			}
			else if (error != WSAEWOULDBLOCK)
			{
				DebuggerPrintf("Error happened at recv()\n");
			}
		}
		else if (receiveResult == 0)
		{
			m_serverState = ServerState::LISTENING;
			Shutdown();
			Startup();
			//DebuggerPrintf("Server receive something to client\n");
			return;
		}

		for(int i = 0; i < (int)m_sendQueue.size(); ++i) 
		{

			if (!strncpy_s(m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen(m_sendQueue[i].c_str()) + 1) == 0) 
				//Copy failed if true
			{
				ERROR_AND_DIE("This copy action is failed");
			}
			if (receiveResult > m_config.m_recvBufferSize)
			{
				ERROR_AND_DIE("the size of send buffer is too big");
			}
			int sendResult = send(m_clientSocket, m_sendBuffer,(int)strlen(m_sendBuffer) + 1, 0);
			if (sendResult == SOCKET_ERROR) 
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					ERROR_AND_DIE("Error happened at send()\n");
				}
			}
		}
		
		m_sendQueue.clear();

		for (int recvCharIndex = 0; recvCharIndex < receiveResult; ++recvCharIndex)
		{
			char const& tempChar = m_recvBuffer[recvCharIndex];
			if (tempChar == -51) 
			{
				break;
			}
			if (tempChar != '\0')
			{
				m_recvRemaining += tempChar;
			}
			else
			{
				if (!m_recvRemaining.empty())
				{
					g_theDevConsole->Execute(m_recvRemaining);
					//std::string const ackBegin = "Server Executed Remote Command: ";
					//std::string const acknowledgement = ackBegin + m_recvRemaining;
					//m_sendQueue.push_back(acknowledgement);
					m_recvRemaining.clear();
				}
				DebuggerPrintf("Server receive something to client\n");
				continue;
			}
		}
		return;
	}

}

void NetSystem::EndFrame()
{

}
//RemoteCommand Command="LoadModel File=Data/Models/Teddy.xml"
bool NetSystem::Event_RemoteCommand(EventArgs& args)
{
	std::string remoteCommand = args.GetValue("Command", "INVALID_REMOTE_COMMAND");
	if (remoteCommand == "INVALID_FILE")
	{
		return false;
	}
	//trim string 
	//and send to queue buffer
	TrimString(remoteCommand,'\"');
	g_theNetSystem->m_sendQueue.push_back(remoteCommand);
	return true;
}

bool NetSystem::Event_BurstTest(EventArgs& args)
{
	UNUSED(args);
	for (int index = 1; index <= 20; ++index) 
	{
		g_theNetSystem->m_sendQueue.emplace_back(Stringf("Echo Message=%d", index));
	}
	return true;
}

void NetSystem::SendMessageCommand(std::string const& command)
{
	m_sendQueue.push_back(command);
}

#endif