#pragma once

class CDNDBConnection;

// Manage global authentication
class CAuthManager
{

public:
	CAuthManager();

	virtual bool Init();

	bool IsResetAuthServer() const { return m_bResetAuthServer; }
	void SetResetAuthServer(bool bResetAuthServer) { m_bResetAuthServer = bResetAuthServer; }

	void Run(DWORD CurTick);

	INT64 GetCertifyingKey();
	int GetServerID() const { return m_ServerID; }

protected:
	virtual int QueryResetAuthServer() = 0;

private:
	const int CalculateServerID();
	char* GetLocalIp(OUT in_addr* pInAddr = NULL);

protected:
	bool m_bResetAuthServer;		// Whether 'auth info reset (specific server)' was performed on initial server startup
	DWORD m_ResetAuthServerTick;	// Manage 'auth info reset' task interval on initial server startup (unit:ms)

	int m_ServerID;					// Unique ID for each server
};
