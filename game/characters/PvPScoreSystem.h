
#pragma once

#include "ScoreSystem.h"

#if defined( _GAMESERVER )
class CDNDBConnection;
class CPvPGameMode;
#endif // #if defined( _GAMESERVER )

class CPvPScoreSystem:public IScoreSystem
{
public:

	CPvPScoreSystem();
	virtual ~CPvPScoreSystem(){}

	virtual bool						IsPvPScoreSystem(){ return true; }

	virtual void						ClearAllScore();
	virtual void						OnFinishRound( DnActorHandle hActor, const bool bIsWin );
	virtual bool						AddUserScore( DnActorHandle hActor, const void* pData );
#if !defined( _GAMESERVER )
	virtual bool						SetCount( DnActorHandle hActor, const void* pData );
#endif
	virtual IPVPGameModeScoreSystem*	CreatePvPGameModeScoreSystem( const UINT uiGameMode );
	// Count-related
	virtual UINT						GetKillCount( DnActorHandle hActor );
	virtual UINT						GetKillCount( const WCHAR * pName );
	virtual UINT						GetDeathCount( DnActorHandle hActor );
	// Score related
	virtual UINT						GetTotalScore( DnActorHandle hActor );
	virtual UINT						GetKillScore( DnActorHandle hActor );
	virtual UINT						GetAssistScore( DnActorHandle hActor );
	virtual UINT						GetXPScore( DnActorHandle hActor, const UINT uiWinTeam );
	virtual const void*					GetScoreStruct( const WCHAR* pwszCharName );
	virtual UINT GetOccupationAcquireScore(DnActorHandle hActor) { return 0; }
	virtual bool GetOccupationTeamScore(int nTeam, int &nTeamScore) { return false; }
	virtual UINT GetOccupationStealScore(DnActorHandle hActor ) { return 0; }
	virtual bool GetOccupationTeamScore(int nTeam, TPvPOccupationTeamScoreInfo &TeamScore) { return false; };
	//
	virtual void						OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage );
	virtual void						OnDie( DnActorHandle hActor, DnActorHandle hHitter );
	virtual void						OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo );
	virtual void						OnLeaveUser( DnActorHandle hActor );
	virtual void						OnNotify( DnActorHandle hActor, const UINT uiScoreType, const UINT uiScore );
	virtual void						OnStartRound();
	virtual bool OnTryAcquirePoint(DnActorHandle hActor, int AreaID, LOCAL_TIME Localtime) { return false; }
	virtual bool OnAcquirePoint(int nTeam, const WCHAR * pName, bool bSnatched = false, bool bBreakInto = false) { return false; }
	virtual void OnOccupationTeamScore(int nTeam, int nType) {}
#if defined(PRE_ADD_REVENGE) && defined(_GAMESERVER)
	void								OnRevengeSuccess(DnActorHandle hHitter);
#endif

#ifdef _GAMESERVER
	virtual void SendScore( CDNUserSession* pGameSession );
	virtual void SendXPScore( CDNGameRoom* pGameRoom, CDNUserSession* pGameSession );
	virtual bool QueryUpdatePvPData( const UINT uiWinTeam, CDNUserSession* pGameSession, PvPCommon::QueryUpdatePvPDataType::eCode Type );
	virtual int	 GetModeExp( DnActorHandle hActor ){ return 0; }
#if defined(PRE_MOD_PVP_LADDER_XP)
	virtual UINT CalcXPScore( CDNUserSession* pGameSession, const UINT uiWinTeam, PvPCommon::QueryUpdatePvPDataType::eCode Type );	
#endif
#endif

#ifdef _CLIENT
	virtual bool IsRedBossKill()	{ return false; }
	virtual bool IsBlueBossKill()	{ return false; }
#endif	//#ifdef _CLIENT

#if defined (_GAMESERVER)
	virtual bool InitializeOccupationSystem(TBattleGourndModeInfo * pModeInfo)	{ return true; }
#endif		//#if defined (_GAMESERVER)
#if defined(_CLIENT)
	virtual void InitializeOccupationSystem()	{ return; }
#endif	// #if defined(_CLIENT)

private:

	typedef struct tagClassScore
	{
		UINT	uiScore[PvPCommon::Common::MaxClass];

		tagClassScore()
		{
			for( UINT i=0 ; i<PvPCommon::Common::MaxClass ; ++i )
				uiScore[i] = 0;
		}
	}SClassScore;

public:

	typedef struct tagMyScore
	{
#ifdef _GAMESERVER
		UINT		uiSessionID;
#endif
		UINT		uiKillScore;
		UINT		uiAssistScore;
		SClassScore	sKillClassScore;
		SClassScore	sKilledClassScore;
#ifdef _GAMESERVER
		UINT		uiWinRound;
		UINT		uiLoseRound;
		UINT		uiAddXP;
		UINT		uiGainMedalCount;
#if defined(PRE_ADD_REVENGE)
		bool		bRevengeSucess;
		tagMyScore( const UINT _uiSessionID ):uiSessionID(_uiSessionID),uiKillScore(0),uiAssistScore(0),uiWinRound(0),uiLoseRound(0),uiAddXP(0),uiGainMedalCount(0), bRevengeSucess(false){}
#else
		//
		tagMyScore( const UINT _uiSessionID ):uiSessionID(_uiSessionID),uiKillScore(0),uiAssistScore(0),uiWinRound(0),uiLoseRound(0),uiAddXP(0),uiGainMedalCount(0){}
#endif

#else
		tagMyScore():uiKillScore(0),uiAssistScore(0){}
#endif // #ifdef _GAMESERVER

		UINT	GetScore(){ return uiKillScore+uiAssistScore; }
	}SMyScore;

	SMyScore*	FindMyScoreData( const WCHAR* pwszName );
	SMyScore*	FindMyScoreDataIfnotInsert( DnActorHandle hActor );

#if defined( _GAMESERVER )
	void		PushQueryCharacterDBID( INT64 biCharDBID ){ m_listQueryCharacterDBID.push_back(biCharDBID); }
	bool		bIsQueryCharacterDBID( INT64 biCharDBID ){ return std::find(m_listQueryCharacterDBID.begin(), m_listQueryCharacterDBID.end(), biCharDBID) != m_listQueryCharacterDBID.end(); }
#endif // #if defined( _GAMESERVER )
	virtual UINT GetMyOccupationScore(const WCHAR * pName) { return 0; }
private:

	typedef struct tagCount
	{
#ifdef _GAMESERVER
		UINT	uiSessionID;
#endif
		UINT	uiKillCount;
		UINT	uiDieCount;						// Death count
		UINT	uiContinuousKillCount;			// Kill streak count
		//
#ifdef _GAMESERVER
		tagCount( const UINT _uiSessionID, UINT _uiKillCount, UINT _uiDieCount, UINT _uiContinuousKillCount ):uiSessionID(_uiSessionID),uiKillCount(_uiKillCount),uiDieCount(_uiDieCount),uiContinuousKillCount(_uiContinuousKillCount){}
#else
		tagCount( UINT _uiKillCount, UINT _uiDieCount, UINT _uiContinuousKillCount ):uiKillCount(_uiKillCount),uiDieCount(_uiDieCount),uiContinuousKillCount(_uiContinuousKillCount){}
#endif
	}SCount;

	std::map<std::wstring,SMyScore>		m_mScore;
	std::map<std::wstring,SCount>		m_mCount;

#ifdef _GAMESERVER
	bool				m_bCreateXPScorePacket;
	SCPVP_XPSCORE		m_XPScorePacket;
	std::list<INT64>	m_listQueryCharacterDBID;
#endif

private:

	void		_UpdateCount( DnActorHandle hActor, DnActorHandle hHitter );
#if defined( _GAMESERVER )
	void		_UpdateAssistPoint( DnActorHandle hActor, DnActorHandle hHitter );
	bool		_QueryUpdatePvPData( CDNDBConnection* pDBCon, CDNUserSession* pGameSession, UINT uiWinTeam, PvPCommon::QueryUpdatePvPDataType::eCode Type );
	bool		_QueryUpdateLadderData( CDNDBConnection* pDBCon, CDNUserSession* pGameSession, UINT uiWinTeam, PvPCommon::QueryUpdatePvPDataType::eCode Type );
	void		_QueryPvPEndLog( CDNDBConnection* pDBCon, CDNUserSession* pGameSession, CPvPGameMode* pPvPGameMode, SMyScore* pMyScore, PvPCommon::QueryUpdatePvPDataType::eCode Type );
#endif // #if defined( _GAMESERVER )
	UINT		_GetDieCount( DnActorHandle hActor );
	UINT		_GetContinuousKillCount( DnActorHandle hActor );
};

class IBoostPoolPvPScoreSystem : public CPvPScoreSystem, public TBoostMemoryPool<IBoostPoolPvPScoreSystem>
{
public :
	//
};
