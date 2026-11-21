
#include "PvPScoreSystem.h"

#pragma once

class COccupationScoreSystem : public CPvPScoreSystem
{
public:
	COccupationScoreSystem();
	virtual ~COccupationScoreSystem();

	//Get
	virtual UINT GetOccupationAcquireScore(DnActorHandle hActor);
	virtual UINT GetOccupationStealScore(DnActorHandle hActor );

	//On
	virtual void OnDamage(DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage);
	virtual void OnDie(DnActorHandle hActor, DnActorHandle hHitter);
	virtual bool OnTryAcquirePoint(DnActorHandle hActor, int AreaID, LOCAL_TIME Localtime);
	virtual void OnLeaveUser( DnActorHandle hActor );
	virtual void OnOccupationTeamScore(int nTeam, int nType);
#if defined( _GAMESERVER )
	virtual void SendScore( CDNUserSession* pGameSession );
#endif
	virtual UINT GetMyOccupationScore(const WCHAR * pName) { return 0; }

protected:
	typedef struct tagOccupationCount
	{
		USHORT nTryAcquireCount;
		USHORT nAcquireCount;
		BYTE cBossKillCount;
		USHORT nStealAcquireCount;

		tagOccupationCount() : nTryAcquireCount(0), nAcquireCount(0), cBossKillCount(0), nStealAcquireCount(0)
		{
		}		
	}SMyOccupationCount;

	typedef struct tagOccupationTeamScore
	{
		int nTeamID;
		int nKillScore;
		int nAcquireScore;
		int nStealAquireScore;		//count

		tagOccupationTeamScore() : nTeamID(0), nKillScore(0), nAcquireScore(0), nStealAquireScore(0)
		{
		}
	}STeamOccupationScore;

	std::map<std::wstring, SMyOccupationCount> m_mOccupationScore;
	std::vector <STeamOccupationScore> m_vOccupationScoreTeam;

	virtual SMyOccupationCount * GetMyOccupationCount(const WCHAR * pName);
	bool AddMyOccupationScore(const WCHAR * pName);
	void AddOccupationTeamScore(DnActorHandle hActor, int nType);
	virtual void AddOccupationTeamScore(int nTeam, int nType) = 0;

#if defined( _GAMESERVER )
	void _SendOccupationScore( CDNUserSession* pSession );
#endif // #if defined( _GAMESERVER )
};
