// RTSP_Demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "RTSPServer.h"
#include "thread"
#include "CycleQueue.h"
#include "VideoCodec.h"
#include "fstream"
#include "GdImageLib.h"
#include "Common.h"

void DataSourceFunc(CCycleQueue *dataBuffer)
{
	char *filePath;

	int frameNum;

	std::ifstream in(filePath);

	in.open(filePath, std::ios::in);

	char *buffer = new char[ImageSize];

	int isFull = 0;

	while (true)
	{
		in.read(buffer, ImageSize);

		if (in.fail() == true)
		{
			break;
		}
		else
		{
			frameNum++;
			dataBuffer->PushBack(reinterpret_cast<unsigned char *>(buffer), ImageSize, isFull);
		}

		in.seekg(frameNum*ImageSize);
	}
	delete[] buffer;
	in.close();
}


void DataProcessFunc(CCycleQueue *bufferSource,CCycleQueue *bufferDst)
{
	ImageChange *m_imageChange = new ImageChange(ImageWidth, ImageHeight, ImageWidth, ImageHeight, AV_PIX_FMT_GRAY8, AV_PIX_FMT_YUV420P);

	unsigned char *imageY16 = new unsigned char[ImageSize];

	unsigned char *imageY8 = new unsigned char[ImageWidth*ImageHeight];

	unsigned char *imageYUV = new unsigned char[ImageWidth*ImageHeight*1.5];

	int size = ImageSize;

	int isFull = 0;
	while (true)
	{
		if (bufferSource->IsEmpty() == false)
		{
			bufferSource->GetFront(imageY16, size);

			GdImageLib::Map16BitTo8Bit_u(
				reinterpret_cast<unsigned short*>(imageY16),
				ImageWidth*ImageHeight,
				imageY8);

			m_imageChange->ConvertFmt(imageY8, imageYUV);

			bufferDst->PushBack(imageYUV, ImageWidth*ImageHeight*1.5, isFull);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	delete[] imageY16;
	delete[] imageY8;
	delete[] imageYUV;
	delete[] m_imageChange;
}

void DataEncodecFun(CCycleQueue *bufferSource, Queue_S<NALUH264Packet> *buffer)
{
	VideoEncodec *videoEncodc = new VideoEncodec();

	videoEncodc->InitEncodec(ImageWidth, ImageHeight, true);

	unsigned char *imageYUV = new unsigned char[ImageWidth*ImageHeight*1.5];

	unsigned char *imageEncodec = new unsigned char[ImageWidth*ImageHeight*1.5];

	int size = ImageWidth*ImageHeight*1.5;

	int frameSize = 0;

	int frameNum = 0;

	while (true)
	{
		if (bufferSource->IsEmpty() == false)
		{
			bufferSource->GetFront(imageYUV, size);

			videoEncodc->WriteEncodecFrame(reinterpret_cast<uint8_t*>(imageYUV), reinterpret_cast<uint8_t*>(imageEncodec), frameSize, frameNum);

			frameNum++;

			NALUH264Packet *Packet = new NALUH264Packet(frameSize);

			Packet->PushData(imageEncodec, frameSize);

			buffer->Push(Packet);
		}
	}
}

int main()
{
	//char addr[] = "rtsp://127.0.0.1";

	std::string temp = "127.0.0.1";

	CRTSPServer *m_server = new CRTSPServer(temp, 8554);

	/************************************************************************/
	/*								图像源读取								*/
	/************************************************************************/
	CCycleQueue *m_cDataSourceBuffer = new CCycleQueue();
	m_cDataSourceBuffer->InitQueue(ImageSize, 50);
	std::thread *m_stDataSourceThread = new std::thread(DataSourceFunc, m_cDataSourceBuffer);
	/************************************************************************/
	/*								图像转换处理								*/
	/************************************************************************/
	CCycleQueue *m_cDataDstBuffer = new CCycleQueue();
	m_cDataDstBuffer->InitQueue(ImageWidth*ImageHeight*1.5, 50);
	std::thread *m_stDataProcessThread = new std::thread(DataProcessFunc, m_cDataSourceBuffer, m_cDataDstBuffer);
	/************************************************************************/
	/*								图像编码处理								*/
	/************************************************************************/
	Queue_S<NALUH264Packet> *m_pNALUBuffer = new Queue_S<NALUH264Packet>();
	std::thread *m_stDataEncodecThread = new std::thread(DataEncodecFun, m_cDataDstBuffer, m_pNALUBuffer);
	/************************************************************************/
	/*								图像封包处理								*/
	/************************************************************************/
	while (1);

    return 0;
}

