#include "stdafx.h"
#include "RTPService.h"


CRTPService::CRTPService()
{
}


CRTPService::CRTPService(NALUH264PacketQueue *buffer)
{
	m_pOutputNaluQueue = new NALUH264PacketQueue();
}

void CRTPService::AddData(NALUH264Packet *packet)
{
	uint8_t *buffer = new uint8_t[ImageWidth*ImageHeight*1.5];

	packet->GetData(buffer);

	for (int i = 0; i < packet->GetSize(); i++)
	{
		m_svDataBuffer.push_back(buffer[i]);
	}

	delete[] buffer;

	std::vector<uint8_t>::iterator beginItem = m_svDataBuffer.begin();

	std::vector<uint8_t>::iterator startItem;

	while (beginItem != m_svDataBuffer.end())
	{
		//解析00 00 00 01的NALU
		if (((*beginItem) == 0x00 && (*(beginItem + 1) == 0x00) && (*(beginItem + 2) == 0x00) && (*(beginItem + 3) == 0x01)))
		{
			if ((beginItem != m_svDataBuffer.begin())&&(beginItem != (m_svDataBuffer.begin() + 1)))
			{
				uint8_t *temp = new uint8_t[(beginItem - startItem)];

				std::vector<uint8_t>::iterator item = beginItem;

				for (int i = 0; i < (beginItem - startItem); i++)
				{
					temp[i] = (*(item));
					item++;
				}

				NALUH264Packet *packetTemp = new NALUH264Packet((beginItem - startItem));

				//设置当前包为 00 00 00 01类型的NAL
				packetTemp->SetStartType(false);

				packetTemp->PushData(temp, (beginItem - startItem));

				m_pOutputNaluQueue->PushData(packetTemp);

				delete[] temp;

				m_svDataBuffer.erase(startItem, beginItem + 1);

				//进行删除操作后，迭代器启始失效
				beginItem = m_svDataBuffer.begin();
			}
			else
			{
				startItem = beginItem;
			}
		}
		else if (((*beginItem) == 0x00 && (*(beginItem + 1) == 0x00) && (*(beginItem + 2) == 0x00) && (*(beginItem + 3) == 0x00) && (*(beginItem + 4) == 0x01)))
		{
			if (beginItem != m_svDataBuffer.begin())
			{
				uint8_t *temp = new uint8_t[(beginItem - startItem)];

				std::vector<uint8_t>::iterator item = beginItem;

				for (int i = 0; i < (beginItem - startItem); i++)
				{
					temp[i] = (*(item));
					item++;
				}

				NALUH264Packet *packetTemp = new NALUH264Packet((beginItem - startItem));

				//设置当前包为00 00 00 00 01类型的NAL
				packetTemp->SetStartType(true);

				packetTemp->PushData(temp, (beginItem - startItem));

				m_pOutputNaluQueue->PushData(packetTemp);

				delete[] temp;

				m_svDataBuffer.erase(startItem, beginItem + 1);

				//进行删除操作后，迭代器起始标志失效，进行重新获取
				beginItem = m_svDataBuffer.begin();
			}
			else
			{
				startItem = beginItem;
			}
		}

		beginItem++;
	}
}

void CRTPService::SetStatus(bool status)
{
	std::unique_lock<std::mutex> lock(m_lock);

	m_bRunStatus = status;
}

CRTPService::~CRTPService()
{
}

void CRTPService::RtpPacketGenerate(uint32_t timeStamp)
{
	while (m_bRunStatus)
	{
		if (m_pOutputNaluQueue->GetSize() != 0)
		{
			NALUH264Packet *h264Packet = m_pOutputNaluQueue->GetData();

			if (h264Packet->GetSize() >= 1400)
			{
				RtpPacketGenerate_FUs(h264Packet, timeStamp);
			}
			else
			{
				RtpPacketGenerate_Signal(h264Packet, timeStamp);
			}

			delete h264Packet;

		}
	}
}

void CRTPService::RtpPacketGenerate_Signal(NALUH264Packet *packet, uint32_t timeStamp)
{
	RTPPacket_Signal *rtpPacket = new RTPPacket_Signal();

	rtpPacket->header.V = 0x2;

	rtpPacket->header.P = 0x0;

	rtpPacket->header.X = 0x0;

	rtpPacket->header.CC = 0x0;

	rtpPacket->header.M = 0x0;

	rtpPacket->header.PT = 0x60;

	m_uiFrameSquence++;

	rtpPacket->header.seqNumber = m_uiFrameSquence;

	rtpPacket->header.timeStamp = timeStamp;

	rtpPacket->header.SSRC = 0x00000000;

	rtpPacket->header.CSRC = 0x00000000;

	uint8_t *buffer = new uint8_t[packet->GetSize()];

	packet->GetData(buffer);

	if (packet->GetStartType())
	{
		rtpPacket->data = new uint8_t[packet->GetSize() - 5];

		memcpy(rtpPacket->data, buffer + 5, packet->GetSize() - 5);

		uint8_t *data = new uint8_t[packet->GetSize() - 5 + 16];

		memcpy(data, &(rtpPacket->header), 16);

		memcpy(data + 16, rtpPacket->data, packet->GetSize() - 5);

		m_sqRtpPacketQueue.push(data);
	}
	else
	{
		rtpPacket->data = new uint8_t[packet->GetSize() - 4];

		memcpy(rtpPacket->data, buffer + 4, packet->GetSize() - 4);

		uint8_t *data = new uint8_t[packet->GetSize() - 4 + 16];

		memcpy(data, &(rtpPacket->header), 16);

		memcpy(data + 16, rtpPacket->data, packet->GetSize() - 4);

		m_sqRtpPacketQueue.push(data);
	}

	delete[] buffer;

	delete packet;

	delete rtpPacket;
}

void CRTPService::RtpPacketGenerate_FUs(NALUH264Packet *packet, uint32_t timeStamp)
{
	int bufferCount = packet->GetSize();

	bool isStart = true;

	uint8_t *buffer = new uint8_t[packet->GetSize()];

	packet->GetData(buffer);

	uint8_t NALHead = buffer[5];

	while (bufferCount >1400)
	{
		RTPPacket_FUs *rtpPacket = new RTPPacket_FUs();

		rtpPacket->header.V = 0x2;

		rtpPacket->header.P = 0x0;

		rtpPacket->header.X = 0x0;

		rtpPacket->header.CC = 0x0;

		rtpPacket->header.M = 0x0;

		rtpPacket->header.PT = 0x60;

		m_uiFrameSquence++;

		rtpPacket->header.seqNumber = m_uiFrameSquence;

		rtpPacket->header.timeStamp = timeStamp;

		rtpPacket->header.SSRC = 0x00000000;

		rtpPacket->header.CSRC = 0x00000000;

		if (packet->GetStartType())
		{
			uint8_t *temp =  (uint8_t*)(&(rtpPacket->indicator));

			(*temp) = ((*temp) & 0xE0) | (NALHead & 0xE0);

			temp = (uint8_t*)(&(rtpPacket->fuheader));

			(*temp) = ((*temp) & 0x1F) | (NALHead & 0x1F);

			if (isStart)
			{
				rtpPacket->fuheader.S = 0x1;
			}
			else
			{
				rtpPacket->fuheader.S = 0x0;
			}

			rtpPacket->fuheader.E = 0x0;

			rtpPacket->fuheader.R = 0x0;

			rtpPacket->indicator.TYPE = 28;

		}
	}
}
