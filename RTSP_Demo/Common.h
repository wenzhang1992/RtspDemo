#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__
#include <vector>
#include <queue>
#include <share.h>
#include <mutex>
#define ImageWidth 1280
#define ImageHeight 720
#define ImagePixelSize 2
#define ImageSize ImageHeight*ImageWidth*ImagePixelSize
#define ImageSizeYUV ImageHeight *ImageWidth * 1.5
typedef struct tag_RTPHeader
{
	char V : 2;
	char P : 1;
	char X : 1;
	char CC : 4;
	char M : 1;
	char PT : 7;
	INT16 seqNumber : 16;
	INT32 timeStamp : 32;
	INT32 SSRC : 32;
	INT32 CSRC : 32;

}RTPHeader;

typedef struct tag_FUIndicator
{
	char F : 1;
	char NRI : 2;
	char TYPE : 5;
}FUIndicator;

typedef struct tag_FUHeader
{
	char S : 1;
	char E : 1;
	char R : 1;
	char TYPE : 5;
}FUHeader;

typedef struct tag_RtpPacketSignal
{
	RTPHeader header;

	uint8_t *data = nullptr;
}RTPPacket_Signal;

typedef struct tag_RtpPacketFUs
{
	FUIndicator indicator;

	FUHeader fuheader;

	RTPHeader header;

	uint8_t *data = nullptr;

}RTPPacket_FUs;

class NALUH264Packet
{
private:
	//std::mutex m_lock;

	int size;

	//判断H264的NAL头是否为 00 00 00 00 01
	bool isFiveStart = false;

	//uint8_t data[ImageWidth*ImageHeight*1.5];
	uint8_t *data = new uint8_t[ImageWidth*ImageHeight*1.5];
public:
	NALUH264Packet()
	{

	}

	~NALUH264Packet()
	{
		delete[] data;
	}

	NALUH264Packet(int size)
	{
		this->size = size;

		data = new uint8_t[ImageWidth*ImageHeight*1.5];
	}

	void SetStartType(bool isFiveStart)
	{
		this->isFiveStart = isFiveStart;
	}

	bool GetStartType()
	{
		return this->isFiveStart;
	}

	void PushData(uint8_t *buffer, int size)
	{
		memcpy(data, buffer, size);
		//data = buffer;
	}

	void GetData(uint8_t *buffer)
	{
		memcpy(buffer, data, size);
	}

	int GetSize()
	{
		return size;
	}
};

class NALUH264PacketQueue
{
private:
	std::queue<NALUH264Packet *> m_sQueue;

	std::mutex m_lock;

public:
	NALUH264PacketQueue()
	{

	}

	~NALUH264PacketQueue()
	{
		for (int i = 0; i < m_sQueue.size(); i++)
		{
			NALUH264Packet *item = m_sQueue.front();

			m_sQueue.pop();

			delete item;
		}
	}
	
	void PushData(NALUH264Packet *obj)
	{
		std::unique_lock<std::mutex> lock(m_lock);

		m_sQueue.push(obj);
	}

	int GetSize()
	{
		return m_sQueue.size();
	}

	NALUH264Packet* GetData()
	{
		if (m_sQueue.size() <= 0)
		{
			return nullptr;
		}
		else
		{
			std::unique_lock<std::mutex> lock(m_lock);

			NALUH264Packet *ret = m_sQueue.front();

			m_sQueue.pop();

			return ret;
		}
	}
};

#endif