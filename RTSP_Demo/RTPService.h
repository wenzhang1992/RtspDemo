#pragma once
#include "VideoCodec.h"
#include "Windows.h"
#include "Common.h"


class CRTPService
{
public:

	CRTPService();

	void AddData(NALUH264Packet *packet);

	void SetStatus(bool status);

	~CRTPService();
private:
	bool m_bRunStatus = false;

	std::mutex m_lock;

	//后台线程，处理H264的封包操作
	std::thread *m_stRtpPacketThread = nullptr;

	Queue_S<NALUH264Packet> *m_pOutputNaluQueue = nullptr;

	std::vector<uint8_t> m_svDataBuffer;	

	//对H264的NAL进行RTP封包处理

	uint16_t m_uiFrameSquence = 0;

	std::queue<uint8_t*> m_sqRtpPacketQueue;

	void RtpPacketGenerate(uint32_t timeStamp);

	//单一H264封包操作
	void RtpPacketGenerate_Signal(NALUH264Packet *packet, uint32_t timeStamp);

	//FUs模式H264封包处理
	void RtpPacketGenerate_FUs(NALUH264Packet *packet, uint32_t timeStamp);
};

