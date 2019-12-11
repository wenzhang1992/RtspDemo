#include "stdafx.h"
#include "RTSPServer.h"

CRTSPServer::CRTSPServer()
{
}


CRTSPServer::CRTSPServer(std::string address,int port)
{
	m_rtspProtocol = new CRTSPProtocol();

	//³õÊ¼ÍøÂç¿â
	WSAData wsa;

	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "init net error"<<"\n";
	}

	m_pIPAddress = new char[address.length()];

	strcpy(m_pIPAddress, const_cast<char*>(address.c_str()));

	m_iRTSPPort = port;

	m_sRTSPSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_sRTSPSocket == INVALID_SOCKET)
	{
		std::cout << "create socket error"<<"\n";
	}

	sockaddr_in sockAddr;

	memset(&sockAddr, 0, sizeof(sockaddr_in));

	sockAddr.sin_family = PF_INET;

	sockAddr.sin_addr.s_addr = inet_addr(m_pIPAddress);

	sockAddr.sin_port = htons(port);

	int ret = bind(m_sRTSPSocket, (sockaddr*)&sockAddr, sizeof(SOCKADDR));

	if (ret == SOCKET_ERROR)
	{
		std::cout << "bind error" << "\n";
	}

	m_pucRTSPBuffer = new char[RTSP_MAX_BUFFER_SIZE];

	m_pRTSPThread = new std::thread(&CRTSPServer::RTSPReceiveProcess,this);

	if (listen(m_sRTSPSocket, 20) != 0)
	{
		std::cout << "listen error" << "\n";
	}
}

CRTSPServer::~CRTSPServer()
{

}

bool CRTSPServer::ConnectBuildStatus()
{
	return m_rtspProtocol->getBuildStatus();
}

void CRTSPServer::InitRTCPSocket()
{
	m_sRTCPSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP);

	sockaddr_in sockAddr;

	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = PF_INET;

	sockAddr.sin_addr.s_addr = inet_addr(m_pIPAddress);

	sockAddr.sin_port = htons(m_iRTCPPort);

	m_pucRTCPBuffer = new char[RTCP_MAX_BUFFER_SIZE];

	bind(m_sRTSPSocket, (sockaddr*)&sockAddr, sizeof(SOCKADDR));
}

void CRTSPServer::InitRTPSocket()
{
	m_sRTPSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	m_sRTPAddress = new sockaddr_in();

	memset(m_sRTPAddress, 0, sizeof(sockaddr_in));

	m_sRTPAddress->sin_family = PF_INET;

	m_sRTPAddress->sin_addr.s_addr = inet_addr(m_pIPAddress);

	m_sRTPAddress->sin_port = htons(m_rtspProtocol->m_rtspParameter.clientRtpPort);

	m_pucRTPBuffer = new char[RTP_MAX_BUFFER_SIZE];

	m_pRTPService = new CRTPService();

	std::thread *m_pRTPThread = new std::thread(RTPTransmitProcess, this);
}

void CRTSPServer::RTSPReceiveProcess(void *pOwner)
{
	CRTSPServer *pUser = reinterpret_cast<CRTSPServer*>(pOwner);

	pUser->m_bRTSPThreadControl = true;

	SOCKET clientSocket;

	//sockaddr *clientAddr = new sockaddr();

	//int clienAddrLength = 0;
	bool isConnection = false;

	while (pUser->m_bRTSPThreadControl)
	{
		
		if (isConnection == false)
		{
			clientSocket=accept(pUser->m_sRTSPSocket, NULL, NULL);
		}

		if (clientSocket == INVALID_SOCKET)
		{
			continue;
		}
		else
		{
			isConnection = true;
		}

		memset(pUser->m_pucRTSPBuffer,0x00 ,RTSP_MAX_BUFFER_SIZE);

		int recvCount = recv(clientSocket, pUser->m_pucRTSPBuffer, RTSP_MAX_BUFFER_SIZE, 0);

		if (recvCount <= 0)
		{
			continue;
		}

		std::cout << "receive:\r\n";

		std::cout << pUser->m_pucRTSPBuffer << "\r\n";

		char *dataBuffer = new char[recvCount];

		memcpy(dataBuffer, pUser->m_pucRTSPBuffer, recvCount);
		
		if (pUser->m_rtspProtocol->analyseOption(dataBuffer, recvCount))
		{
			char *sBuf = pUser->m_rtspProtocol->respondOption();

			if (pUser->m_icurrentSessionStatus == -1)
			{
				int sCount = send(clientSocket, sBuf, strlen(sBuf), 0);

				std::cout << "send:" << "\r\n";
				std::cout << sBuf << "\r\n";

				pUser->m_icurrentSessionStatus = OPTIONS;

				std::cout << "OPTION SUCCESS" << "\r\n";
			}
						
			delete[] sBuf;			
		}
		else if (pUser->m_rtspProtocol->analyseDescribe(dataBuffer, recvCount))
		{
			char* sBuf = pUser->m_rtspProtocol->respondDescribe(pUser->m_pIPAddress);

			if (pUser->m_icurrentSessionStatus == OPTIONS)
			{
				int sCount = send(clientSocket, sBuf, strlen(sBuf), 0);
				
				std::cout << "send:" << "\r\n";
				std::cout << sBuf << "\r\n";
				std::cout << "DESCRIBE SUCCESS" << "\r\n";

				pUser->m_icurrentSessionStatus = DESCRIBE;
			}		

			delete[] sBuf;		

		}
		else if (pUser->m_rtspProtocol->analyseSetup(dataBuffer, recvCount))
		{
			char *sBuf = pUser->m_rtspProtocol->respondSetup();

			if (pUser->m_icurrentSessionStatus == DESCRIBE)
			{
				int sCount = send(clientSocket, sBuf, strlen(sBuf), 0);
				std::cout << "send:" << "\r\n";
				std::cout << sBuf << "\r\n";
				std::cout << "SETUP SUCCESS" << "\r\n";
				pUser->m_icurrentSessionStatus = SETUP;
			}
		
			delete[] sBuf;
			
		}
		else if (pUser->m_rtspProtocol->analysePlay(dataBuffer, recvCount))
		{
			char* sBuf = pUser->m_rtspProtocol->respondPlay();

			pUser->InitRTPSocket();

			if (pUser->m_icurrentSessionStatus == SETUP)
			{
				int sCount = send(clientSocket, sBuf, strlen(sBuf), 0);
				std::cout << "send:" << "\r\n";
				std::cout << sBuf << "\r\n";
				std::cout << "PLAY SUCCESS" << "\r\n";
				pUser->m_icurrentSessionStatus = PLAY;


			}		
			delete[] sBuf;
		}
		else
		{
			std::cout << "ANALYSE FAIL" << "\r\n";
		}
		delete[] dataBuffer;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

}

void CRTSPServer::RTPTransmitProcess(void *pUser)
{
	CRTSPServer *pOwner = reinterpret_cast<CRTSPServer*>(pUser);

	while (true)
	{
		if (pOwner->m_rtspProtocol->getBuildStatus())
		{
			VideoPacket *packet = pOwner->m_pRTPService->GetPacket();

			if (packet == nullptr)
			{
				continue;
			}

			int ret = sendto(pOwner->m_sRTPSocket, (char *)packet->data, packet->size, 0, (sockaddr *)(pOwner->m_sRTPAddress), sizeof(sockaddr_in));

			if (ret < 0)
			{
				int error = WSAGetLastError();
				std::cout << "send fail" << "\r\n";
			}
			else
			{
				std::cout << "send rtp packet" << "\r\n";
			}

			delete packet;		
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}
