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
	����OPETIONS����
	@param ���ݻ�����
	@param ����������
	*/
	bool analyseOption(char *buffer, int length);

	/*
	��ӦOPETIONS����
		RTSP/1.0 200 OK\r\n
		CSeq: 2\r\n
		Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n
		\r\n
	*/
	char* respondOption();

	/*
	����SETUP����
	@param ���ݻ�����
	@param ����������
	*/
	bool analyseSetup(char *buffer, int length);

	/*
	��ӦSETUP����
		RTSP/1.0 200 OK\r\n
		CSeq: 4\r\n
		Transport: RTP/AVP;unicast;client_port=cRtpPort-cRtcpPort;server_port=sRtpPort-sRtcpPort\r\n
		Session: sessionID\r\n
		\r\n
	*/
	char* respondSetup();

	/*
	����DESCRIBE����
	@param:���ݻ�����
	@param:����������
	*/
	bool analyseDescribe(char *buffer, int length);

	/*
	��ӦDESCRIBE����
	*/
	char* respondDescribe(char *servAddr);
	/*
	����SDP�ַ���
	*/
	char* generateSDP(char *serverAddr);

	/*
	����PLAY����
	@param:���ݻ�����
	@param:����������
	*/
	bool analysePlay(char *buffer, int length);

	/*
	��ӦPLAY����
	*/

	char* respondPlay();
	
	RTSPParameter m_rtspParameter;

	bool getBuildStatus();

private:
	std::mutex m_lock;

	bool m_bBuildSuceess = false;

	SDPData m_sdpData;

	/*
		��ȡ������ָ���е�����
		@param ���ݻ�����
		@param �����������
		@param �������ĳ���
		@param ָ������
	*/
	void getLineData(char *buffer, char* rowData, int length, int row);

};

