#include "stdafx.h"
#include "RTSPProtocol.h"
#include "string.h"

CRTSPProtocol::CRTSPProtocol()
{
}


CRTSPProtocol::~CRTSPProtocol()
{
}

bool CRTSPProtocol::analyseOption(char *buffer, int length)
{
	char lineData[BUFFER_SIZE] = {0};

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 0);

	sscanf(lineData, "%s %s\r\n", m_rtspParameter.currentOption, m_rtspParameter.currentUrl);

	//delete[] lineData;

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 1);

	sscanf(lineData, "CSeq: %d\r\n", &(m_rtspParameter.currentCseq));

	//delete[] lineData;

	if (strcmp((m_rtspParameter.currentOption), "OPTIONS\0")==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

char* CRTSPProtocol::respondOption()
{
	char *sBuf = new char[BUFFER_SIZE];

	memset(sBuf, 0x00, BUFFER_SIZE);

	sprintf(sBuf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
		"\r\n",
		m_rtspParameter.currentCseq);

	return sBuf;
}

bool CRTSPProtocol::analyseSetup(char *buffer, int length)
{
	char *lineData = new char[BUFFER_SIZE];

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 0);

	sscanf(lineData, "%s %s\r\n", m_rtspParameter.currentOption, m_rtspParameter.currentUrl);

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 1);

	sscanf(lineData, "CSeq: %d\r\n", &(m_rtspParameter.currentCseq));

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 3);

	sscanf(lineData, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", &(m_rtspParameter.clientRtpPort), &(m_rtspParameter.clientRtcpPort));

	delete[] lineData;

	if (strcmp(m_rtspParameter.currentOption, "SETUP\0")==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

char* CRTSPProtocol::respondSetup()
{
	char *sBuf = new char[BUFFER_SIZE];

	memset(sBuf, 0x00, BUFFER_SIZE);

	sprintf(sBuf, "RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Transport: RTP/AVP;unicast;client_port=%d-%d;server-port=%d-%d\r\n"
		"Session: 9%ld\r\n"
		"\r\n",
		m_rtspParameter.currentCseq,
		m_rtspParameter.clientRtpPort,
		m_rtspParameter.clientRtcpPort,
		m_rtspParameter.serverRtpPort,
		m_rtspParameter.serverRtcpPort,
		m_sdpData.sessionID);
	return sBuf;
}

bool CRTSPProtocol::analyseDescribe(char *buffer, int length)
{
	char lineData[BUFFER_SIZE] = {0};

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 0);

	sscanf(lineData, "%s %s\r\n", m_rtspParameter.currentOption,m_rtspParameter.currentUrl);

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 1);

	sscanf(lineData, "CSeq: %d\r\n", &(m_rtspParameter.currentCseq));

	//delete[] lineData;

	if (strcmp(m_rtspParameter.currentOption, "DESCRIBE\0")==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

char* CRTSPProtocol::respondDescribe(char *servAddr)
{
	char *sBuf = new char[BUFFER_SIZE];

	memset(sBuf, 0x00, BUFFER_SIZE);

	char *sdpData = generateSDP(servAddr);

	sprintf(sBuf, 
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-length: %d\r\n"
		"Content-Base: rtsp://%s\r\n"
		"\r\n"
		"%s",
		m_rtspParameter.currentCseq,
		strlen(sdpData),
		servAddr,
		sdpData);

	delete[] sdpData;

	return sBuf;
}

char* CRTSPProtocol::generateSDP(char *serverAddr)
{
	//char sdpData[BUFFER_SIZE] = {0};
	char *sdpData = new char[1024];

	memset(sdpData, 0x00, 1024);

	sprintf(sdpData,
		"v=0\r\n"
		"o=- 9%ld 1 IN IP4 %s\r\n"
		"c=IN IP4 0.0.0.0\r\n"
		"t=0 0\r\n"
		"a=control:*\r\n"
		"m=video 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 H264/90000\r\n"
		"a=framerate:25\r\n"
		"a=control:trackID=0\r\n",
		m_sdpData.sessionID,
		serverAddr);
	
	return sdpData;
}

bool CRTSPProtocol::analysePlay(char *buffer, int length)
{
	char *lineData = new char[BUFFER_SIZE];

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 0);

	sscanf(lineData, "%s %s\r\n", m_rtspParameter.currentOption, m_rtspParameter.currentUrl);

	memset(lineData, 0x00, BUFFER_SIZE);

	getLineData(buffer, lineData, length, 1);

	sscanf(lineData, "CSeq: %d\r\n", &(m_rtspParameter.currentCseq));

	if (strcmp(m_rtspParameter.currentOption, "PLAY\0")==0)
	{
		std::unique_lock<std::mutex> locker(m_lock);

		m_bBuildSuceess = true;

		return true;
	}
	else
	{
		return false;
	}
}

char* CRTSPProtocol::respondPlay()
{
	char *sBuf = new char[BUFFER_SIZE];

	sprintf(sBuf, "RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Range: npt=0.000-\r\n"
		"Session: 9%ld; timeout=60\r\n"
		"\r\n",
		m_rtspParameter.currentCseq,
		m_sdpData.sessionID);

	return sBuf;
}

bool CRTSPProtocol::getBuildStatus()
{
	std::unique_lock<std::mutex> locker(m_lock);

	return this->m_bBuildSuceess;
}

void CRTSPProtocol::getLineData(char *buffer, char* rowData, int length, int row)
{
	int rowTemp = 0;

	int beginPos = 0;

	int endPos = 0;

	for (int i = 0; (i + 1) < length; i++)
	{
		if (buffer[i] == '\r' && buffer[i + 1] == '\n')
		{
			if (rowTemp == row)
			{
				endPos = i+2;
				break;
			}
			else
			{
				beginPos = i+2;
				rowTemp++;
			}
		}
	}

	memcpy(rowData, buffer+beginPos, (endPos-beginPos));
}
