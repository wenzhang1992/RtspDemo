#pragma once
#include "VideoCodec.h"
#include "Windows.h"
#include "Common.h"
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

class CRTPService
{
public:
	CRTPService();

	CRTPService(NALUH264PacketQueue *buffer);

	void AddData(NALUH264Packet *packet);

	~CRTPService();
private:
	NALUH264PacketQueue *m_pInputNaluQueue = nullptr;

	NALUH264PacketQueue *m_pOutputNaluQueue = nullptr;

	std::vector<uint8_t> m_svDataBuffer;
};

