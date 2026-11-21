#pragma once

#include "DNGameRoom.h"


class CDnFarmGameTask;

class CDNFarmGameRoom : public CDNGameRoom, public TBoostMemoryPool<CDNFarmGameRoom>
{
public:
	
	CDNFarmGameRoom( CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket );
	virtual ~CDNFarmGameRoom();

	//Parse Database
	virtual void OnDBMessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen);

	//Query data
	void QueryGetListField();

	//Update
	virtual void FarmUpdate();

	// CallBack
	void OnStartGuildWarFinal();

	//GetFarmData;
	int GetFarmMaxUser() { return m_nFarmMaxUser; }

	virtual void	OnInitGameRoomUser();
	virtual void	OnSync2Sync( CDNUserSession* pBreakIntoSession );
	virtual bool	bIsFarmRoom(){ return true; }
	virtual void	SetFarmActivation(bool bActivation);
	virtual void	OnLeaveUser( const UINT uiSessionID );
	virtual void	UpdateCharacterName(MAChangeCharacterName* pPacket);

	void BroadCast( int iMainCmd, int iSubCmd, char* pData, int iLen );

	CDnFarmGameTask* GetFarmGameTask(){ return reinterpret_cast<CDnFarmGameTask*>(GetGameTask()); }

#if defined( PRE_ADD_FARM_DOWNSCALE )
	int GetAttr(){ return m_iAttr; }
	void AddAttr( Farm::Attr::eType Attr ){ m_iAttr |= Attr; }
	void DelAttr( Farm::Attr::eType Attr ){ m_iAttr &= ~Attr; }
#elif defined( PRE_ADD_VIP_FARM )
	Farm::Attr::eType GetAttr(){ return m_Attr; }
#endif // #if defined( PRE_ADD_VIP_FARM )

protected:

private:
	int m_nFarmMaxUser;
	int m_nReportedUserCount;
	bool m_bFarmStartFlag;
	bool m_bReportedFarmStartFlag;
#if defined( PRE_ADD_FARM_DOWNSCALE )
	int m_iAttr;
#elif defined( PRE_ADD_VIP_FARM )
	Farm::Attr::eType m_Attr;
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	bool m_bSendFarmDataLoadedFail;

	void OnDBRecvFarm(int nSubCmd, char * pData, int nLen);
};
