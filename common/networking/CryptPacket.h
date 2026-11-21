
#pragma once
#include "DNPacket.h"
#include "MtRandom.h"

// 2009.01.19 kimbap
// Define DNGAME_PACKET header size
const int DNGAME_PACKET_HEADERSIZE = 7;

class CCryptoPacket
{
public:
	CCryptoPacket();
	virtual ~CCryptoPacket() {}

	int EncodeGamePacket(void *p, int main_header, int sub_header, const void *message, int size, BYTE cSeq = 0);
	bool DecodeGamePacket(DNGAME_PACKET *p);
	int CalcGamePacketSize(void *p, int len); // -1 (unknown) if too little has been read yet
};

class CCryptoKey
{
public:
	CCryptoKey();
	~CCryptoKey() {}

	//for server
	bool ServerMakeKey(char * pKey, bool bCheck);
	int ServerVerifyKey(char * pKey);

	//for client
	bool ClientVerifyKey(char * pKey);

private:
	CMtRandom m_Random;
	bool m_bMakeKey;

	bool MakeCryptoKey(char * pBuf);
	int VerifyCryptoKey(char * pBuf, bool bFullLenth);
};
