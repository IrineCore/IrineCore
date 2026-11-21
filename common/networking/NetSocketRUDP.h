
#pragma once

#include <winsock.h>
#include "RUDPCommonDef.h"
#include "RUDPSocketFrame.h"
#include "CryptPacket.h"
#include "Buffer.h"

//class CRUDPReceiver;
class CSeqReceiver;
class CNetSocketRUDP : public CRUDPSocketFrame , public CCryptoPacket
{
public:
	CNetSocketRUDP();
	~CNetSocketRUDP();
	
	bool Connect(const char * ip, int port);
	bool Connect(unsigned long ip, int port);
	void DisConnect();
	void DisConnected(int iNetID, bool bForce, bool bUnreachable);

	int Send(int main_header, int sub_header, void * data, int size, int prior, BYTE cSeqLevel);
	void Receive(void * data, int size, _ADDR * addr);

	void DetectAddr(const char * ip, int port);
	void SetReceiver(CSeqReceiver * pReceiver) { m_pReceiver = pReceiver; }

	void GetAddr(ULONG * pIP, USHORT * pPort) { *pIP = m_UDPIP; *pPort = m_UDPPort; }

protected:
	bool Accept(int id, SOCKADDR_IN * addr) { return false; }		//External access not allowed
	void Recv(int id, void * data, int len);

private:
	friend class CNetSocketUDP;

	struct _TEMP_UDPPACKET
	{
		USHORT nSize;
		_ADDR addr;
		BYTE cSeq;
		DNGAME_PACKET packet;
	};

	_ADDR m_DirectAddr;
	_ADDR m_DetectAddr;

	int m_hNetID;
	int SendTo(void * msg, int size, int prior);

	//My Addr
	unsigned long m_UDPIP;
	unsigned short m_UDPPort;

	//detect addr
	bool Accept(int id, SOCKADDR_IN * addr, const void * buf, int size);
	CSeqReceiver * m_pReceiver;
	bool m_bIdle;
};