#pragma once
#include <mutex>
#define OPTIONS 0
#define DESCRIBE 1
#define SETUP 2
#define PLAY 3

#define BUFFER_SIZE 1024

typedef struct tag_RTSPParameter {

	char currentOption[10] = {0};

	char currentUrl[100] = {0};

	int currentCseq = 0;

	int clientRtcpPort = 54493;
	
	int clientRtpPort = 54492;

	int serverRtcpPort = 54493;

	int serverRtpPort = 54492;

}RTSPParameter;

typedef struct tag_SDPData {
	char username = '-';
	long sessionID = 91565340853;
	int sessionVersion = 1;
	char intentType[2] = {'I','N'};
	char addrType[3] = { 'I','P','4' };
	char *addr = new char[20];
}SDPData;

class CRTSPProtocol
{
public:
	CRTSPProtocol();

	~CRTSPProtocol();

	/*
	解析OPETIONS命令
	@param 数据缓存区
	@param 缓存区长度
	*/
	bool analyseOption(char *buffer, int length);

	/*
	响应OPETIONS命令
		RTSP/1.0 200 OK\r\n
		CSeq: 2\r\n
		Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n
		\r\n
	*/
	char* respondOption();

	/*
	解析SETUP命令
	@param 数据缓存区
	@param 缓存区长度
	*/
	bool analyseSetup(char *buffer, int length);

	/*
	响应SETUP命令
		RTSP/1.0 200 OK\r\n
		CSeq: 4\r\n
		Transport: RTP/AVP;unicast;client_port=cRtpPort-cRtcpPort;server_port=sRtpPort-sRtcpPort\r\n
		Session: sessionID\r\n
		\r\n
	*/
	char* respondSetup();

	/*
	解析DESCRIBE命令
	@param:数据缓存区
	@param:缓存区长度
	*/
	bool analyseDescribe(char *buffer, int length);

	/*
	回应DESCRIBE命令
	*/
	char* respondDescribe(char *servAddr);
	/*
	生成SDP字符串
	*/
	char* generateSDP(char *serverAddr);

	/*
	解析PLAY命令
	@param:数据缓存区
	@param:缓冲区长度
	*/
	bool analysePlay(char *buffer, int length);

	/*
	回应PLAY命令
	*/

	char* respondPlay();
	
	RTSPParameter m_rtspParameter;

	bool getBuildStatus();

private:
	std::mutex m_lock;

	bool m_bBuildSuceess = false;

	SDPData m_sdpData;

	/*
		获取数据中指定行的数据
		@param 数据缓存区
		@param 输出的行数据
		@param 缓存区的长度
		@param 指定行数
	*/
	void getLineData(char *buffer, char* rowData, int length, int row);

};

