
#pragma once
#pragma warning( disable:4996 )

#define	_PACKET_TYPE(A)			((A) & 7)
#define _PACKET_SEQ(A)			(unsigned short)((A) & (~7))
#define _PACKET_ACK(A)			(unsigned short)((A) & (~7))
#define _PACKET_PINGCNT(A)		(unsigned short)((A) & (~7))
#define _PACKET_ACKN(A)			((A)>>3)
#define _PACKET_HEADER(T, S)	((T) | (S))
#define PING_TEST_COUNT	4	//	4 test runs (use average)

#pragma pack(1)
struct _PACKET_QUEUE
{
	union {
		unsigned short seq;
		unsigned short ack;
	} ;
	char type;
	unsigned short len;
	unsigned long tick;
	unsigned long origintick;
	char data[1];
} ;

struct	_RELIABLE_UDP_HEADER
{
	union {
		unsigned short combo;
		unsigned char	flags; // 0x01 => ackreq, 0x02 => seq, 0x03 => ping, 0x06 => pong, 0x04 => ack, 0x05 => ack seq 0x00 => NULL
		unsigned short ack;
		unsigned short seq;
		unsigned short acknum;
	} ;
	unsigned char	crc;
} ;
#pragma pack()

enum {
	_FAST,							//Just fire away~
	_RELIABLE_NOORDER,				//Order doesn't matter, just make sure to send it reliably
	_RELIABLE,						//Order matters and make sure to send it reliably
} ;

struct _ADDR {
	int port;
	unsigned char ip[4];
};

static unsigned long _inet_addr(const char * cp)
{
	int s_b[4] = { 0, 0, 0, 0};
	sscanf(cp, "%d.%d.%d.%d", &s_b[0], &s_b[1], &s_b[2], &s_b[3]);
	unsigned char b[4] = {
		static_cast<unsigned char>(s_b[0]),
		static_cast<unsigned char>(s_b[1]),
		static_cast<unsigned char>(s_b[2]),
		static_cast<unsigned char>(s_b[3])
	};
	unsigned int addr = 0;
	memcpy(&addr, b, 4);
	return static_cast<unsigned long>(addr);
}

static unsigned long _inet_addr(const WCHAR * cp)
{
	int s_b[4] = { 0, 0, 0, 0};
#if defined(_WIN32)
	swscanf(cp, L"%d.%d.%d.%d", &s_b[0], &s_b[1], &s_b[2], &s_b[3]);
#else
	if (cp) {
		std::string narrow = Irine::Platform::WideToUtf8(cp);
		sscanf(narrow.c_str(), "%d.%d.%d.%d", &s_b[0], &s_b[1], &s_b[2], &s_b[3]);
	}
#endif
	unsigned char b[4] = {
		static_cast<unsigned char>(s_b[0]),
		static_cast<unsigned char>(s_b[1]),
		static_cast<unsigned char>(s_b[2]),
		static_cast<unsigned char>(s_b[3])
	};
	unsigned int addr = 0;
	memcpy(&addr, b, 4);
	return static_cast<unsigned long>(addr);
}

static unsigned long _inet_addr(const unsigned char * ip)
{
	unsigned int addr = 0;
	memcpy(&addr, ip, 4);
	return static_cast<unsigned long>(addr);
}

static void _inet_addr(unsigned long i, char * p)
{
	sprintf(p, "%d.%d.%d.%d", ((unsigned char*)&i)[0], ((unsigned char*)&i)[1], ((unsigned char*)&i)[2], ((unsigned char*)&i)[3]);
}