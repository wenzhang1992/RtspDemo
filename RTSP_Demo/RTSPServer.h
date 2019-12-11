#pragma once

#include <WinSock2.h>
#include <thread>
#include <iostream>
#include "RTSPProtocol.h"
#include "RTPService.h"
#pragma comment (lib,"ws2_32.lib")

#define RTSP_MAX_BUFFER_SIZE 4028

#define RTCP_MAX_BUFFER_SIZE 4028

#define RTP_MAX_BUFFER_SIZE 4028

class CRTSPServer
{
public:

	CRTSPServer();

	CRTSPServer(std::string address,int port);

	~CRTSPServer();

	bool ConnectBuildStatus();

	CRTPService* GetRtp()
	{
		return this->m_pRTPService;
	}

	void InitRTPSocket();

	void InitRTCPSocket();
private:
	CRTSPProtocol *m_rtspProtocol = nullptr;

	char *m_pIPAddress = nullptr;

	int m_iRTSPPort = 0;

	int m_iRTCPPort = 54493;

	int m_iRTPPort = 54492;

	//RTSP套接字
	SOCKET m_sRTSPSocket;
	//RTCP套接字
	SOCKET m_sRTCPSocket;
	//RTP套接字
	SOCKET m_sRTPSocket;

	sockaddr_in *m_sRTPAddress;

	//接收线程
	//RTSP
	int m_icurrentSessionStatus = -1;

	char *m_pucRTSPBuffer = nullptr;

	std::thread *m_pRTSPThread = nullptr;

	bool m_bRTSPThreadControl = false;

	static void RTSPReceiveProcess(void *pOwner);
	
	//RTCP
	char *m_pucRTCPBuffer = nullptr;

	std::thread *m_pRTCPThread = nullptr;

	//RTP
	CRTPService *m_pRTPService = nullptr;

	char *m_pucRTPBuffer = nullptr;

	std::thread *m_pRTPThread = nullptr;

	static void RTPTransmitProcess(void *pUser);
};

