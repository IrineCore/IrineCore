
#pragma once

#include <winsock.h>
#include "Thread.h"
#include "CryptPacket.h"
#include "RUDPCommonDef.h"
#include "CryptPacket.h"
#include "Buffer.h"

class CNetSocketRUDP;
class CRUDPReceiver;
class CNetSocketUDP : public Thread, public CCryptoPacket
{
public:
	CNetSocketUDP(const char * szNick, CNetSocketRUDP * pSocket);
	~CNetSocketUDP();

	void DetectAddr(const char * ip, int port);
	void SetReciever(CRUDPReceiver * pReciever);
	
	bool Create(int port = 0);
	int Send(unsigned int main_header, unsigned int sub_header, void * data, int size, unsigned long addr, unsigned short port);
	void Receive(void * data, int size, _ADDR * addr);

	unsigned short GetPort();
	unsigned long GetIP();
private:
	int SendTo(void * data, int size, SOCKADDR_IN * addr);
	void SetDirection(unsigned long iIP, unsigned short iPort);

	void Run();

	SOCKADDR_IN m_DirectionAddr;
	SOCKET m_hSocket;

	_ADDR m_DetectAddr;
	unsigned long m_UDPIP;
	unsigned short m_UDPPort;

	CRUDPReceiver * m_pReciever;
	CNetSocketRUDP * m_pNetSocket;
};