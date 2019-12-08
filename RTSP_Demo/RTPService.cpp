#include "stdafx.h"
#include "RTPService.h"


CRTPService::CRTPService()
{
}


CRTPService::CRTPService(NALUH264PacketQueue *buffer)
{
	m_pInputNaluQueue = buffer;

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

		beginItem++;
	}
}

CRTPService::~CRTPService()
{
}
