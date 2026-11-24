
#include "stdafx.h"
#include "CryptPacket.h"
#include <mmsystem.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

static unsigned char _table[2][8] = {
	6, 2, 7, 0, 1, 3, 5, 4,
	3, 4, 1, 5, 7, 6, 0, 2,
} ;


static unsigned char _mix_bytes[13] = {
	0x3f, 0x26, 0x42, 0xfd, 0x8a, 0x72, 0x34, 0x74, 0xc8, 0xbf, 0xcd, 0xa4, 0x94,
} ;


inline unsigned char EncodeGamePacketSize(unsigned char codes)
{
	//	CCCb bAAA
	//	AAAC CCbb
	return (_table[0][codes&7] << 5) | ((codes>>3)&3) | (_table[0][(codes>>5)&7] << 2);
}


inline unsigned char DecodeGamePacketSize(unsigned char codes)
{
	//	AAAC CCbb
	//	CCCb bAAA
	return _table[1][(codes>>5)&7] | (_table[1][(codes>>2)&7] << 5) | ((codes&3)<<3);
}

CCryptoPacket::CCryptoPacket()
{
}

int CCryptoPacket::EncodeGamePacket(void *p, int main_header, int sub_header, const void *message, int size, BYTE cSeq)
{
	int i, total;
	unsigned char magickey, encoded_main_header, encoded_sub_header;
	DNGAME_PACKET * packet = (DNGAME_PACKET*) p;
	
	encoded_main_header = EncodeGamePacketSize(main_header);
	encoded_sub_header = EncodeGamePacketSize(sub_header);

	packet->seq = EncodeGamePacketSize(cSeq);
	packet->datasize = size;

	packet->header = magickey = (unsigned char)encoded_main_header ^ _mix_bytes[size%sizeof(_mix_bytes)];
	packet->sub_header = (unsigned char)encoded_sub_header ^ _mix_bytes[size%sizeof(_mix_bytes)];
	for(i=0, total=0; i<size; i++)
		packet->data[i] = ((char*)message)[i] ^ _mix_bytes[(magickey+i+7)%sizeof(_mix_bytes)];
	
	packet->checksum = DNGAME_PACKET_HEADERSIZE + size;
	return DNGAME_PACKET_HEADERSIZE + size;
}

bool CCryptoPacket::DecodeGamePacket(DNGAME_PACKET *p)
{
	unsigned short size;
	unsigned char magickey, header, sub_header, seq;
	int i, total;

	size = p->datasize;
	seq = p->seq;
	magickey = p->header;
	header = magickey ^ _mix_bytes[size%sizeof(_mix_bytes)];
	sub_header = p->sub_header ^ _mix_bytes[size%sizeof(_mix_bytes)];	

	p->header = DecodeGamePacketSize(header);
	p->sub_header = DecodeGamePacketSize(sub_header);
	p->seq = DecodeGamePacketSize(seq);

	for(i=0, total=0; i<size; i++)
		p->data[i] ^= _mix_bytes[(magickey+i+7)%sizeof(_mix_bytes)];
	
	return size + DNGAME_PACKET_HEADERSIZE == p->checksum ? true : false;
}

int CCryptoPacket::CalcGamePacketSize(void *p, int len)
{
	if( len >= DNGAME_PACKET_HEADERSIZE )
		return static_cast<DNGAME_PACKET*>(p)->datasize + DNGAME_PACKET_HEADERSIZE;

	return sizeof(DNGAME_PACKET);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
const static char * s_pSzSample1 = { "I'm riding around, This cheaters town, Every place I go, Just bringin' me down I don't know when, I don't know if, \
									I'm ever comin' home I got my broken heart, Torn all in two, Cause every place I look, I picture him and you I dont know when, \
									I dont know if, I'm ever comin home Yeah baby you lied to me, you lied to me You stood there you smiled you opened your heart and you lied I dont know when, \
									I dont know if, I'm ever comin home, Never comin' home I'm drivin' slow, Don't know where I'm goin', I thought you loved me too, But I was always alone \
									I don't know when, I don't know if, I'm ever comin home, Comin' home I got a broken heart, Torn all in two, Cause everything I had, \
									I gave it all to you I don't know when, I don't know if, I'm ever comin' home, Comin' home, Comin' home Oh baby you lied to me, you lied to me \
									You opened up your little black heart you smiled and you lied I don't know when, I don't know if, I'm ever comin' home, Oh I don't know when, I don't know if, \
									I'm ever comin' home, Oh I don't know when, I don't know if, I'm ever comin' home, Comin' home " };

const static char * s_pSzSample2 = { "Down a dirt road in southern Louisiana Flew a white Continental with the top laid back Pretty blonde sittin' on a seat of red leather \
									 Dust blowin' outta those well wore tracks She goin' back Moss hangin' down like beards in the bayou Old folks watchin' from a rockin' chair\
									 Ain't no place for a girl of her state She's gonna ruin her reputation She don't care cause he's waitin' there She's gonna say Tell me you think that I'm pretty\
									 Tell me you think that I'm smart Tell me anything and I'll believe it'Cause I'm listening with my heart Maybe it's wrong but that's the way it goes\
									 When the only love you get is down Cheater's road Bedsprings rubbin' her back through the blanket Sweat like a tear rollin' down her neck\
									 Watchin' him pullin' down the shades on the window She'd rather have him than an empty bed and her self respect And she says\
									 Tell me you think that I'm pretty Tell me you think that I'm smart Tell me anything and I'll believe it'Cause I'm listening with my heart\
									 Maybe it's wrong but that's the way it goes When the only love you get is down Cheater's road Old man countin' his money in the city\
									 Nothin' but the bottom dollar on his mind The whole situation is a downright pity He didn't even notice when she crossed that line\
									 When she crossed that line And she said Tell me you think that I'm pretty Tell me you think that I'm smart Tell me anything and I'll believe \
									 it'Cause I'm listening with my heart Maybe it's wrong but that's the way it goes When the only love you get is down Cheater's road\
									 Down cheater's road Oh tell me you think that I'm pretty Tell me you think that I'm smart Oh tell me you think that I'm pretty\
									 Tell me you think that I'm smart Oh tell me you think that I'm pretty Tell me you think that I'm smart" };

CCryptoKey::CCryptoKey()
{
	m_Random.srand(timeGetTime());
	m_bMakeKey = false;
}

bool CCryptoKey::ServerMakeKey(char * pKey, bool bCheck)
{
	if (m_bMakeKey && bCheck) return false;
	bool bRet = MakeCryptoKey(pKey);
	m_bMakeKey = true;
	return bRet;
}

int CCryptoKey::ServerVerifyKey(char * pKey)
{
	if (m_bMakeKey == false) return -1;				//never maked serverkey
	int nRet = VerifyCryptoKey(pKey, true);
	m_bMakeKey = false;
	return nRet;
}

bool CCryptoKey::ClientVerifyKey(char * pKey)
{
	if (VerifyCryptoKey(pKey, false) == 0)
		return MakeCryptoKey(pKey + 9);
	return false;
}

bool CCryptoKey::MakeCryptoKey(char * pBuf)
{
	if (pBuf == NULL) return false;

	int nCnt = 0;
	BYTE cSampleKey = m_Random.rand(UCHAR_MAX);	
	const char * pSampleKey = cSampleKey%2 == 0 ? s_pSzSample1 : s_pSzSample2;
	pBuf[nCnt++] = EncodeGamePacketSize(cSampleKey);
	
	USHORT nSampleKeyLen = (USHORT)strlen(pSampleKey);
	short nKey[2];
	for (int i = 0; i < sizeof(nKey)/sizeof(*nKey); i++)
	{
		nKey[i] = m_Random.rand(nSampleKeyLen - 2);
		pBuf[nCnt++] = EncodeGamePacketSize(((unsigned char*)&nKey[i])[0]);
		pBuf[nCnt++] = EncodeGamePacketSize(((unsigned char*)&nKey[i])[1]);
	}

	for (int i = 0; i < sizeof(nKey)/sizeof(*nKey); i++)
	{
		pBuf[nCnt++] = EncodeGamePacketSize(pSampleKey[nKey[i]]);
		pBuf[nCnt++] = EncodeGamePacketSize(pSampleKey[nKey[i]+1]);
	}

	return true;
}

int CCryptoKey::VerifyCryptoKey(char * pBuf, bool bFullLenth)
{
	if (pBuf == NULL) return -3;					//generic error

	int nCnt = 0;
	int nLenth = bFullLenth == true ? 2 : 1;
	for (int j = 0; j < nLenth; j++)
	{
		BYTE cSampleKey = DecodeGamePacketSize(pBuf[nCnt++]);
		const char * pSampleKey = cSampleKey%2 == 0 ? s_pSzSample1 : s_pSzSample2;

		USHORT nSampleKeyLen = (USHORT)strlen(pSampleKey);

		short nKey[2];
		for (int i = 0; i < sizeof(nKey)/sizeof(*nKey); i++)
		{
			((unsigned char*)&nKey[i])[0] = DecodeGamePacketSize(pBuf[nCnt++]);
			((unsigned char*)&nKey[i])[1] = DecodeGamePacketSize(pBuf[nCnt++]);
		}

		for (int i = 0; i < sizeof(nKey)/sizeof(*nKey); i++)
		{
			for (int h = 0; h < 2; h++)
				if (pSampleKey[nKey[i] + h] != DecodeGamePacketSize(pBuf[nCnt++]))
					return -2;					//diff key
		}
	}
	return 0;
}