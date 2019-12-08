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
class NALUH264Packet
{
private:
	std::mutex m_lock;

	int size;

	//uint8_t data[ImageWidth*ImageHeight*1.5];
	uint8_t *data = new uint8_t[ImageWidth*ImageHeight*1.5];
public:
	NALUH264Packet()
	{

	}

	NALUH264Packet(int size)
	{
		this->size = size;

		data = new uint8_t[ImageWidth*ImageHeight*1.5];
	}

	void PushData(uint8_t *buffer, int size)
	{
		std::unique_lock<std::mutex> lock(m_lock);

		memcpy(data, buffer, size);
		//data = buffer;
	}

	void GetData(uint8_t *buffer)
	{
		std::unique_lock<std::mutex> lock(m_lock);

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